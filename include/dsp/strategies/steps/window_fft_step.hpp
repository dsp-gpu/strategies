#pragma once

// ============================================================================
// WindowFftStep — окно Hamming + zero-pad + batch FFT + magnitudes (Ref03-C Step)
//
// ЧТО:    «Толстый» pipeline-шаг, объединяющий 4 операции:
//           1. hipMemsetAsync — занулить kBufFftInput полностью (для zero-padding);
//           2. fused-kernel hamming_pad_fused — наложить окно Hamming на kBufX
//              и записать в первые n_samples элементов kBufFftInput
//              (оставшиеся nFFT - n_samples — нули из шага 1);
//           3. hipfftExecC2C(HIPFFT_FORWARD) — batch FFT kBufFftInput → kBufSpectrum;
//           4. ComplexToMagPhaseROCm::ProcessMagnitudeToBuffer —
//              kBufSpectrum → kBufMagnitudes (|X|, без фазы, zero-alloc).
//         Всё на ctx.stream_main; в конце — hipEventRecord(event_fft_done)
//         для зависимых шагов (DebugStatsStep::POST_FFT, OneMax/AllMaxima/MinMax).
//
// ЗАЧЕМ:  Эти 4 операции жёстко связаны (FFT-pipeline) и переиспользования
//         поодиночке не имеют — оборачивать в один шаг разумно. Если бы
//         каждая была отдельным шагом, пришлось бы тащить ещё 3 hipEvent
//         для синхронизации между ними, при том что на одном stream это
//         не нужно (FIFO-исполнение).
//
// ПОЧЕМУ: - hipMemsetAsync БОЛЬШЕ kBufFftInput (n_ant × nFFT × complex),
//           а fused-kernel пишет ТОЛЬКО первые n_samples — оптимизация P11
//           убирает if-else в kernel'е (без проверки «индекс < n_samples
//           ? data : 0» в каждом thread'е). Memset один раз, fused-kernel
//           без branch'а → лучше для warp execution на gfx1201.
//         - hamming_pad_fused (P13+P10+P6) — фьюжен: окно + копия + pad
//           в одном kernel'е, ОДИН проход по data. Альтернатива (3 kernel'а)
//           = 3× memory bandwidth, что критично на ~MB-данных.
//         - Batch FFT через hipfft plan (был создан фасадом с
//           rank=1, batch=n_ant): один вызов hipfftExecC2C обрабатывает
//           все лучи одновременно, без host-loop.
//         - ComplexToMagPhaseROCm::ProcessMagnitudeToBuffer с третьим
//           параметром {n_ant, nFFT, 0.0f} — последний 0.0f это inv_n
//           норма: 0 значит «без нормировки» (hipfft не нормирует, мы тоже
//           оставляем |X| без 1/N — это контракт API, нормирование на
//           CPU при сравнении с эталоном).
//         - hipEventRecord ПОСЛЕ всех 4 операций — гарантия что
//           dependent шаги (post-FFT) увидят kBufMagnitudes и kBufSpectrum.
//
// Использование:
//   builder.add(std::make_unique<GemmStep>())
//          .add(std::make_unique<WindowFftStep>())            // вот этот
//          .add(std::make_unique<DebugStatsStep>(DebugPoint::POST_FFT));
//
// История:
//   - Создан: 2026-03-14 (Ref03-C, фьюжен трёх старых шагов в один)
// ============================================================================


#include <dsp/strategies/i_pipeline_step.hpp>
#include <dsp/strategies/pipeline_context.hpp>

#include <hip/hip_runtime.h>
#include <hipfft/hipfft.h>
#include <stdexcept>

// Forward declaration
namespace dsp::spectrum { struct MagPhaseParams; }

namespace dsp::strategies {

/**
 * @class WindowFftStep
 * @brief Pipeline-шаг: zero-pad + Hamming + hipfftExecC2C + |X| (zero-alloc).
 *
 * @note IsEnabled = always true (FFT — обязательный этап pipeline'а).
 * @note Все операции на ctx.stream_main; в конце hipEventRecord(event_fft_done).
 * @note Magnitude без нормировки (inv_n=0): hipfft не делит на N, мы тоже не делим.
 * @see GemmStep                              — поставщик kBufX через event_gemm_done.
 * @see ::dsp::spectrum::ComplexToMagPhaseROCm  — реализация magnitude (zero-alloc).
 */
class WindowFftStep : public PipelineStepBase {
public:
  /**
   * @brief Возвращает имя шага для логирования и поиска через Pipeline::FindStep.
   *
   * @return C-строка "WindowFFT" (статический литерал).
   *   @test_check std::string(result) == "WindowFFT"
   */
  const char* Name() const override { return "WindowFFT"; }
  /**
   * @brief Всегда активен — обязательный шаг (window + FFT + magnitudes).
   *
   *
   * @return Всегда `true`.
   *   @test_check result == true
   */
  bool IsEnabled(const AntennaProcessorConfig&) const override { return true; }

  /**
   * @brief Запустить memset → fused window+pad → FFT → magnitudes, записать event_fft_done.
   * @param ctx Shared context: kBufX, kBufHammingWindow, kBufFftInput, kBufSpectrum, kBufMagnitudes,
   *   @test { values=["valid_backend"] }
   *            fft_plan, complex_to_mag, stream_main, event_fft_done.
   * @throws std::runtime_error при ошибке любого kernel/hipfft вызова.
   *   @test_check throws on hipfftExecC2C != HIPFFT_SUCCESS || hipModuleLaunchKernel != hipSuccess
   */
  void Execute(PipelineContext& ctx) override {
    uint32_t n_ant = ctx.cfg->n_ant;
    uint32_t n_samples = ctx.cfg->n_samples;
    uint32_t nFFT = ctx.nFFT;
    uint32_t total_fft = n_ant * nFFT;

    // 1. Zero entire FFT input buffer (P11 — eliminates if-else in kernel)
    hipMemsetAsync(ctx.buf(kBufFftInput), 0,
                   static_cast<size_t>(total_fft) * sizeof(float) * 2,
                   ctx.stream_main);

    // 2. Fused Hamming window + copy (P13+P10+P6)
    {
      unsigned int grid_x = (n_samples + kBlockSize - 1) / kBlockSize;
      unsigned int grid_y = n_ant;
      void* d_X = ctx.buf(kBufX);
      void* d_fft_in = ctx.buf(kBufFftInput);
      void* d_window = ctx.buf(kBufHammingWindow);

      void* args[] = { &d_X, &d_fft_in, &d_window, &n_ant, &n_samples, &nFFT };

      hipError_t err = hipModuleLaunchKernel(
          ctx.gpu_ctx->GetKernel("hamming_pad_fused"),
          grid_x, grid_y, 1,
          kBlockSize, 1, 1,
          0, ctx.stream_main,
          args, nullptr);
      if (err != hipSuccess) {
        throw std::runtime_error("WindowFftStep: hamming_pad_fused failed");
      }
    }

    // 3. Batch FFT: kBufFftInput → kBufSpectrum
    hipfftResult fft_result = hipfftExecC2C(
        ctx.fft_plan,
        static_cast<hipfftComplex*>(ctx.buf(kBufFftInput)),
        static_cast<hipfftComplex*>(ctx.buf(kBufSpectrum)),
        HIPFFT_FORWARD);
    if (fft_result != HIPFFT_SUCCESS) {
      throw std::runtime_error("WindowFftStep: hipfftExecC2C failed");
    }

    // 4. Compute magnitudes (via ComplexToMagPhaseROCm, zero-alloc)
    ctx.complex_to_mag->ProcessMagnitudeToBuffer(
        ctx.buf(kBufSpectrum), ctx.buf(kBufMagnitudes),
        {n_ant, nFFT, 0.0f});

    // Signal FFT completion for dependent steps (debug23, post-fft)
    hipEventRecord(ctx.event_fft_done, ctx.stream_main);
  }

private:
  static constexpr unsigned int kBlockSize = 256;
};

} // namespace dsp::strategies

