#pragma once

// ============================================================================
// GemmStep — взвешивание сигнала: X = W × S через hipBLAS CGEMM (Ref03-C Step)
//
// ЧТО:    Pipeline-шаг GEMM (complex single-precision): умножает входной
//         сигнал d_S [n_samples × n_ant] на матрицу весов d_W [n_ant × n_ant],
//         результат пишет в kBufX. Размерности M=n_samples, N=K=n_ant,
//         alpha=(1,0), beta=(0,0). Stream берётся из hipblas_handle (был
//         привязан к ctx.stream_main при создании). После запуска CGEMM
//         ставит hipEventRecord(event_gemm_done) — это синхро-точка для
//         dependent-шагов (DebugStatsStep::POST_GEMM, WindowFftStep).
//
// ЗАЧЕМ:  Beamforming в РЛС — взвешенное суммирование сигналов антенн с
//         комплексными коэффициентами (формирование луча). Это самая
//         тяжёлая host-side операция в pipeline (после FFT) — отдельный
//         шаг чтобы её можно было профилировать, отключить, заменить
//         (например на rocBLAS вместо hipBLAS) без перестройки pipeline'а.
//
// ПОЧЕМУ: - hipblasCgemm работает В column-major (BLAS-конвенция, унаследовано
//           от Fortran). Наши данные row-major — используем classic-трюк:
//           swap A/B (вместо C=A*B считаем C^T = B^T * A^T, по BLAS это
//           тот же hipblasCgemm с транспонированными измерениями) →
//           leading dim'ы M, K, M. Без свапа пришлось бы либо явно
//           транспонировать буферы (extra kernel), либо передавать
//           HIPBLAS_OP_T (медленнее на gfx1201).
//         - alpha=(1,0), beta=(0,0): чистое умножение, без аккумуляции
//           в выходной буфер. kBufX перезаписывается, не нужен hipMemset.
//         - hipEventRecord ПОСЛЕ Cgemm на том же stream — гарантия что
//           зависимые шаги увидят готовый kBufX (через hipStreamWaitEvent
//           на их streams).
//         - IsEnabled = always true: GEMM — обязательный шаг, без него
//           последующие шаги работают на пустом kBufX.
//
// Использование:
//   builder.add(std::make_unique<GemmStep>())          // в pipeline
//          .add(std::make_unique<DebugStatsStep>(DebugPoint::POST_GEMM));
//   // hipblas_handle и stream_main должны быть в ctx до Execute.
//
// История:
//   - Создан: 2026-03-14 (Ref03-C, выделено из AntennaProcessor::process_gemm)
// ============================================================================

#if ENABLE_ROCM

#include <dsp/strategies/i_pipeline_step.hpp>
#include <dsp/strategies/pipeline_context.hpp>

#include <hip/hip_runtime.h>
#include <hipblas/hipblas.h>
#include <stdexcept>

namespace dsp::strategies {

/**
 * @class GemmStep
 * @brief Pipeline-шаг GEMM: kBufX = d_W × d_S через hipblasCgemm.
 *
 * @note Row-major трюк: swap A/B в column-major hipBLAS (без явного транспонирования).
 * @note Stream неявно через hipblas_handle binding; после Cgemm — hipEventRecord(event_gemm_done).
 * @note IsEnabled = always true (обязательный шаг beamforming'а).
 * @see WindowFftStep      — потребитель kBufX через event_gemm_done.
 * @see DebugStatsStep     — POST_GEMM наблюдение.
 */
class GemmStep : public PipelineStepBase {
public:
  /**
   * @brief Возвращает имя шага для логирования и поиска через Pipeline::FindStep.
   *
   * @return C-строка "GEMM" (статический литерал).
   *   @test_check std::string(result) == "GEMM"
   */
  const char* Name() const override { return "GEMM"; }
  /**
   * @brief Всегда активен — обязательный шаг beamforming'а.
   *
   *
   * @return Всегда `true`.
   *   @test_check result == true
   */
  bool IsEnabled(const AntennaProcessorConfig&) const override { return true; }

  /**
   * @brief Запустить hipblasCgemm и записать event_gemm_done.
   * @param ctx Shared context: hipblas_handle, d_S/d_W, kBufX, stream_main, event_gemm_done.
   *   @test { values=["valid_backend"] }
   * @throws std::runtime_error если hipblasCgemm вернул не SUCCESS.
   *   @test_check throws on hipblasCgemm != HIPBLAS_STATUS_SUCCESS
   */
  void Execute(PipelineContext& ctx) override {
    const int M = static_cast<int>(ctx.cfg->n_samples);
    const int N = static_cast<int>(ctx.cfg->n_ant);
    const int K = static_cast<int>(ctx.cfg->n_ant);

    hipComplex alpha = {1.0f, 0.0f};
    hipComplex beta  = {0.0f, 0.0f};

    // Row-major trick: swap A/B in column-major hipBLAS
    hipblasStatus_t status = hipblasCgemm(
        ctx.hipblas_handle,
        HIPBLAS_OP_N, HIPBLAS_OP_N,
        M, N, K,
        &alpha,
        static_cast<const hipComplex*>(ctx.d_S), M,
        static_cast<const hipComplex*>(ctx.d_W), K,
        &beta,
        static_cast<hipComplex*>(ctx.buf(kBufX)), M);

    if (status != HIPBLAS_STATUS_SUCCESS) {
      throw std::runtime_error("GemmStep: hipblasCgemm failed");
    }

    // Signal GEMM completion for dependent steps (debug22, window_fft)
    hipEventRecord(ctx.event_gemm_done, ctx.stream_main);
  }
};

} // namespace dsp::strategies

#endif  // ENABLE_ROCM
