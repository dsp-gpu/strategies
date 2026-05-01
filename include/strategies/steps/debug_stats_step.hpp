#pragma once

// ============================================================================
// DebugStatsStep — статистика в debug-точках pipeline'а (Ref03-C Step)
//
// ЧТО:    Параметризованный pipeline-шаг с тремя инстансами по точке
//         наблюдения (DebugPoint::PRE_INPUT, POST_GEMM, POST_FFT).
//         В каждой точке: ждёт нужное hipEvent (если есть), вызывает
//         StatisticsProcessor::ComputeStatistics[Float] на соответствующем
//         буфере, опционально считает ComputeMedian (если в маске StatPreset
//         есть STAT_MEDIAN), записывает в result->{pre_input/post_gemm/post_fft}_stats
//         и сохраняет чекпоинт через ICheckpointSave (бинарный dump для
//         offline-анализа в Python).
//
// ЗАЧЕМ:  В DSP-пайплайне РЛС нужно валидировать данные на КАЖДОМ этапе:
//         входной сигнал d_S, после взвешивания d_X (выход GEMM), после FFT
//         |spectrum|. Без debug-точек невозможно отлаживать кросс-модульные
//         проблемы (где «уехали» данные — на входе, после GEMM, в спектре).
//         Один параметризованный класс вместо трёх ScopedStats-классов
//         избегает дублирования кода (event wait + compute + checkpoint
//         идентичны, отличаются только источник данных и event).
//
// ПОЧЕМУ: - Один класс с enum DebugPoint вместо трёх классов: SRP не
//           нарушается, потому что роль одна — «считай stats в точке X»;
//           точка передаётся в конструктор. Три отдельных класса дали бы
//           ~60 строк копипасты на каждый.
//         - hipStreamWaitEvent (НЕ hipEventSynchronize) — non-blocking
//           между host и GPU; шаг ставится на debug-stream, который ждёт
//           основной поток через event. Если в Pipeline это PARALLEL-шаг —
//           debug-вычисления идут параллельно основному, не блокируя GEMM.
//         - PRE_INPUT не ждёт events — d_S уже на GPU из ctx (caller гарантирует).
//         - POST_FFT использует ComputeStatisticsFloat (магнитуды float),
//           PRE_INPUT/POST_GEMM — ComputeStatistics (complex). Разные
//           dispatch'и, выбираются по DebugPoint host-side, не по runtime
//           проверке типа в kernel.
//         - Cross-module: явная зависимость от stats::StatisticsProcessor
//           через PipelineContext (non-owning ptr). Шаг не создаёт
//           processor сам — переиспользует один на pipeline.
//         - ICheckpointSave абстракция: в production — пустой stub,
//           в тестах — пишет в файл. Ноль overhead в release.
//
// Использование:
//   builder.add(std::make_unique<DebugStatsStep>(DebugPoint::PRE_INPUT))
//          .add(std::make_unique<GemmStep>())
//          .add(std::make_unique<DebugStatsStep>(DebugPoint::POST_GEMM))
//          .add(std::make_unique<WindowFftStep>())
//          .add(std::make_unique<DebugStatsStep>(DebugPoint::POST_FFT));
//
// История:
//   - Создан: 2026-03-14 (Ref03-C, объединение трёх debug-шагов в один)
// ============================================================================

#if ENABLE_ROCM

#include <strategies/i_pipeline_step.hpp>
#include <strategies/pipeline_context.hpp>
#include <strategies/config/statistics_set.hpp>
#include <stats/statistics_processor.hpp>

#include <hip/hip_runtime.h>

namespace strategies {

/**
 * @enum DebugPoint
 * @brief Точка наблюдения для DebugStatsStep: до/после GEMM/после FFT.
 */
enum class DebugPoint : uint8_t { PRE_INPUT, POST_GEMM, POST_FFT };

/**
 * @class DebugStatsStep
 * @brief Pipeline-шаг: статистика + median + checkpoint в debug-точке.
 *
 * @note Параметризован DebugPoint в конструкторе — три экземпляра на pipeline.
 * @note Использует hipStreamWaitEvent (non-blocking GPU-sync между streams).
 * @note Cross-module зависимость: stats::StatisticsProcessor через PipelineContext.
 * @see stats::StatisticsProcessor — реализация Welford/median/histogram.
 * @see ICheckpointSave             — абстракция дампа в файл (для offline Python-анализа).
 */
class DebugStatsStep : public PipelineStepBase {
public:
  explicit DebugStatsStep(DebugPoint point) : point_(point) {}

  /// Имя зависит от DebugPoint: DebugStats_{PreInput|PostGEMM|PostFFT}.
  const char* Name() const override {
    switch (point_) {
      case DebugPoint::PRE_INPUT: return "DebugStats_PreInput";
      case DebugPoint::POST_GEMM: return "DebugStats_PostGEMM";
      case DebugPoint::POST_FFT:  return "DebugStats_PostFFT";
    }
    return "DebugStats_Unknown";
  }

  /// Включён если StatPreset для соответствующей точки != NONE.
  bool IsEnabled(const AntennaProcessorConfig& cfg) const override {
    switch (point_) {
      case DebugPoint::PRE_INPUT: return cfg.pre_input_stats != StatPreset::NONE;
      case DebugPoint::POST_GEMM: return cfg.post_gemm_stats != StatPreset::NONE;
      case DebugPoint::POST_FFT:  return cfg.post_fft_stats  != StatPreset::NONE;
    }
    return false;
  }

  /**
   * @brief Считать статистики (+median по флагу) в точке point_, записать в result + checkpoint.
   * @param ctx Shared context: stats_processor, checkpoint, нужные буферы и events.
   *
   * Для POST_GEMM/POST_FFT ставит hipStreamWaitEvent на debug-stream
   * перед запуском вычислений — гарантия что данные готовы. PRE_INPUT
   * не ждёт (d_S уже на GPU по контракту caller'а).
   */
  void Execute(PipelineContext& ctx) override {
    statistics::StatisticsParams sp;
    sp.beam_count = ctx.cfg->n_ant;

    switch (point_) {
      case DebugPoint::PRE_INPUT:
        // No wait needed — d_S is ready
        sp.n_point = ctx.cfg->n_samples;
        ctx.result->pre_input_stats = ctx.stats_processor->ComputeStatistics(
            const_cast<void*>(ctx.d_S), sp);
        if (ctx.cfg->pre_input_stats & STAT_MEDIAN) {
          ctx.result->pre_input_medians = ctx.stats_processor->ComputeMedian(
              const_cast<void*>(ctx.d_S), sp);
        }
        ctx.checkpoint->save_c1_signal(ctx.d_S, ctx.cfg->n_ant,
                                        ctx.cfg->n_samples, ctx.cfg->sample_rate, ctx.gpu_id);
        break;

      case DebugPoint::POST_GEMM:
        // Wait for GEMM to complete
        hipStreamWaitEvent(ctx.stream_debug2, ctx.event_gemm_done, 0);
        sp.n_point = ctx.cfg->n_samples;
        ctx.result->post_gemm_stats = ctx.stats_processor->ComputeStatistics(
            ctx.buf(kBufX), sp);
        if (ctx.cfg->post_gemm_stats & STAT_MEDIAN) {
          ctx.result->post_gemm_medians = ctx.stats_processor->ComputeMedian(
              ctx.buf(kBufX), sp);
        }
        ctx.checkpoint->save_c2_data(ctx.buf(kBufX), ctx.cfg->n_ant,
                                      ctx.cfg->n_samples, ctx.cfg->sample_rate, ctx.gpu_id);
        break;

      case DebugPoint::POST_FFT:
        // Wait for FFT to complete
        hipStreamWaitEvent(ctx.stream_debug3, ctx.event_fft_done, 0);
        sp.n_point = ctx.nFFT;
        ctx.result->post_fft_stats = ctx.stats_processor->ComputeStatisticsFloat(
            ctx.buf(kBufMagnitudes), sp);
        if (ctx.cfg->post_fft_stats & STAT_MEDIAN) {
          ctx.result->post_fft_medians = ctx.stats_processor->ComputeMedianFloat(
              ctx.buf(kBufMagnitudes), sp);
        }
        ctx.checkpoint->save_c3_spectrum(ctx.buf(kBufSpectrum), ctx.cfg->n_ant,
                                          ctx.nFFT, ctx.gpu_id);
        break;
    }
  }

private:
  DebugPoint point_;
};

}  // namespace strategies

#endif  // ENABLE_ROCM
