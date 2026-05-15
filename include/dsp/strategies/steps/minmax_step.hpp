#pragma once

// ============================================================================
// MinMaxStep — глобальные MIN+MAX по лучу за один проход (Ref03-C Step)
//
// ЧТО:    Pipeline-шаг сценария ALL_REQUIRED / GLOBAL_MINMAX. Запускает HIP-
//         kernel global_minmax (1 block × n_ant blocks по второму измерению,
//         256 threads/block): один проход по kBufMagnitudes [n_ant × nFFT]
//         даёт per-beam min, max, их позиции (bin) и dynamic range в дБ.
//         Результат [n_ant × MinMaxResult] копируется D2H в result->minmax,
//         плюс checkpoint через ICheckpointSave.
//
// ЗАЧЕМ:  Для оценки уровня шума и SNR нужны и максимум и МИНИМУМ
//         спектра. Реализация двумя проходами reduce (отдельный max,
//         отдельный min) = 2× memory bandwidth. global_minmax kernel
//         ищет оба за один проход + сразу считает 20*log10(max/min) на
//         GPU — никаких лишних D2H для дБ-расчёта на CPU.
//
// ПОЧЕМУ: - SetStream(s) — позволяет фасаду ставить шаг на любой stream
//           для PARALLEL-группы (post-FFT три шага идут одновременно на
//           stream_post_a/b/c). По умолчанию — stream_debug3 (sequential).
//         - Grid (1, n_ant, 1): каждый y-блок обрабатывает один луч,
//           внутри блока — параллельная reduce-свёртка по nFFT bin'ам
//           (warp shuffle на gfx1201). 1 block × beam, не нужно сдвигать
//           частичные результаты между блоками — сразу запись в final.
//         - hipStreamSynchronize ПЕРЕД hipMemcpy — kernel должен
//           завершиться до D2H (без sync — race condition: D2H читает
//           недогружённый буфер).
//         - sizeof(MinMaxResult)=36 на host vs 32 в device: pad-поле
//           держит alignment, хост-структура совпадает с device-структурой
//           до байта (см. result_types.hpp комментарий «32-byte alignment»).
//
// Использование:
//   auto step = std::make_unique<MinMaxStep>();
//   step->SetStream(ctx.stream_post_c);              // для PARALLEL
//   builder.add(std::move(step));
//
// История:
//   - Создан: 2026-03-14 (Ref03-C, выделено из AntennaProcessor)
// ============================================================================


#include <dsp/strategies/i_pipeline_step.hpp>
#include <dsp/strategies/pipeline_context.hpp>

#include <hip/hip_runtime.h>
#include <stdexcept>

namespace dsp::strategies {

/**
 * @class MinMaxStep
 * @brief Pipeline-шаг: per-beam global MIN+MAX + dynamic range через kernel global_minmax.
 *
 * @note SetStream(s) — для исполнения в PARALLEL-группе (по умолчанию stream_debug3).
 * @note 1 block × n_ant: каждый y-блок обрабатывает один луч (warp-reduce внутри).
 * @see OneMaxStep   — поиск ОДНОГО максимума (без минимума).
 * @see AllMaximaStep — все локальные пики (другая задача).
 */
class MinMaxStep : public PipelineStepBase {
public:
  /**
   * @brief Возвращает имя шага для логирования и поиска через Pipeline::FindStep.
   *
   * @return C-строка "GlobalMinMax" (статический литерал).
   *   @test_check std::string(result) == "GlobalMinMax"
   */
  const char* Name() const override { return "GlobalMinMax"; }

  /**
   * @brief Активен в сценариях ALL_REQUIRED и GLOBAL_MINMAX (поиск глобального min+max).
   *
   * @param cfg Конфиг pipeline'а (scenario_mode определяет активность).
   *   @test_ref AntennaProcessorConfig
   *
   * @return true для ALL_REQUIRED / GLOBAL_MINMAX, иначе false.
   *   @test_check result == (cfg.scenario_mode == PostFftScenarioMode::ALL_REQUIRED || cfg.scenario_mode == PostFftScenarioMode::GLOBAL_MINMAX)
   */
  bool IsEnabled(const AntennaProcessorConfig& cfg) const override {
    return cfg.scenario_mode == PostFftScenarioMode::ALL_REQUIRED ||
           cfg.scenario_mode == PostFftScenarioMode::GLOBAL_MINMAX;
  }

  /// Назначить stream (для PARALLEL-группы); default = ctx.stream_debug3.
  void SetStream(hipStream_t s) { override_stream_ = s; }

  /**
   * @brief Запустить global_minmax kernel, синхронизировать stream и скопировать D2H.
   * @param ctx Shared context: gpu_ctx (kernel), kBufMagnitudes, kBufMinMaxResults, n_ant, nFFT, sample_rate.
   *   @test { values=["valid_backend"] }
   * @throws std::runtime_error если hipModuleLaunchKernel вернул не hipSuccess.
   *   @test_check throws on hipModuleLaunchKernel != hipSuccess
   */
  void Execute(PipelineContext& ctx) override {
    uint32_t n_ant = ctx.cfg->n_ant;
    float sr = ctx.cfg->sample_rate;
    hipStream_t stream = override_stream_ ? override_stream_ : ctx.stream_debug3;

    void* d_mag = ctx.buf(kBufMagnitudes);
    void* d_results = ctx.buf(kBufMinMaxResults);

    void* args[] = { &d_mag, &d_results, &n_ant, &ctx.nFFT, &sr };

    hipError_t err = hipModuleLaunchKernel(
        ctx.gpu_ctx->GetKernel("global_minmax"),
        1, n_ant, 1,
        kBlockSize, 1, 1,
        0, stream,
        args, nullptr);
    if (err != hipSuccess) {
      throw std::runtime_error("MinMaxStep: kernel launch failed");
    }

    hipStreamSynchronize(stream);
    ctx.result->minmax.resize(n_ant);
    hipMemcpy(ctx.result->minmax.data(), d_results,
              n_ant * sizeof(MinMaxResult), hipMemcpyDeviceToHost);

    ctx.checkpoint->save_c3_minmax(ctx.result->minmax.data(), n_ant, ctx.gpu_id);
  }

private:
  hipStream_t override_stream_ = nullptr;
  static constexpr unsigned int kBlockSize = 256;
};

} // namespace dsp::strategies

