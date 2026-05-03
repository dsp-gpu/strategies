#pragma once

// ============================================================================
// AllMaximaStep — поиск ВСЕХ локальных максимумов спектра (Ref03-C Step)
//
// ЧТО:    Pipeline-шаг scenario_mode = ALL_REQUIRED / ALL_MAXIMA. Делегирует
//         работу готовому AllMaximaPipelineROCm (модуль spectrum):
//         читает kBufMagnitudes (|spectrum|) и kBufSpectrum (complex),
//         вызывает Execute(...) с параметрами n_ant/nFFT/sample_rate +
//         search_start=1 (skip DC), search_end=0 (=> nFFT/2), maxima_limit
//         из конфига. Результат (.beams) перекладывается в result->all_maxima.
//
// ЗАЧЕМ:  В сценарии ALL_REQUIRED нужно вернуть все пики выше порога —
//         в отличие от OneMaxStep (один глобальный) и MinMaxStep (только
//         глобальный max). AllMaximaPipelineROCm — отдельная сложная
//         операция в spectrum (находит локальные максимумы, фильтрует по
//         threshold, делает параболическую интерполяцию + извлекает фазу),
//         оборачивать её в этот шаг = переиспользование без дублирования.
//
// ПОЧЕМУ: - Адаптер-делегат: шаг сам не запускает kernel, только маршрутизирует
//           параметры из PipelineContext в AllMaximaPipelineROCm. SRP — шаг
//           отвечает только за интеграцию в Pipeline, алгоритм — в spectrum.
//         - Указатель all_maxima_pipeline в ctx — non-owning (владелец
//           Facade), это намеренно: pipeline может переиспользоваться
//           между process()-вызовами без пересоздания (init/release дорогой).
//         - search_start=1 — пропуск DC-компоненты (она почти всегда max
//           по случайным причинам, не сигнал).
//         - OutputDestination::CPU — финальный результат на host (фасад
//           отдаёт его Python через pybind), GPU-копию здесь не оставляем.
//
// Использование:
//   builder.add(std::make_unique<AllMaximaStep>());   // в pipeline
//   // далее всё автоматически: IsEnabled проверит scenario_mode, Execute
//   // прочитает ctx.buf(kBufMagnitudes/kBufSpectrum), запишет в ctx.result.
//
// История:
//   - Создан: 2026-03-14 (Ref03-C, выделено из AntennaProcessor)
// ============================================================================

#if ENABLE_ROCM

#include <strategies/i_pipeline_step.hpp>
#include <strategies/pipeline_context.hpp>

#include <spectrum/pipelines/all_maxima_pipeline_rocm.hpp>

namespace strategies {

/**
 * @class AllMaximaStep
 * @brief Pipeline-шаг: все локальные максимумы спектра через AllMaximaPipelineROCm.
 *
 * @note Делегат — сам kernel не запускает; алгоритм в spectrum/AllMaximaPipelineROCm.
 * @note search_start=1 (skip DC); search_end=0 = автоматический половинный диапазон nFFT/2.
 * @see OneMaxStep    — поиск ОДНОГО максимума (быстрее).
 * @see MinMaxStep    — глобальные min+max (без локальных пиков).
 * @see antenna_fft::AllMaximaPipelineROCm — реализация алгоритма.
 */
class AllMaximaStep : public PipelineStepBase {
public:
  /**
   * @brief Возвращает имя шага для логирования и поиска через Pipeline::FindStep.
   *
   * @return C-строка "AllMaxima" (статический литерал).
   *   @test_check std::string(result) == "AllMaxima"
   */
  const char* Name() const override { return "AllMaxima"; }

  /**
   * @brief Активен в сценариях ALL_REQUIRED и ALL_MAXIMA (поиск всех локальных пиков).
   *
   * @param cfg Конфиг pipeline'а (scenario_mode определяет активность шага).
   *   @test_ref AntennaProcessorConfig
   *
   * @return true для ALL_REQUIRED / ALL_MAXIMA, иначе false.
   *   @test_check result == (cfg.scenario_mode == PostFftScenarioMode::ALL_REQUIRED || cfg.scenario_mode == PostFftScenarioMode::ALL_MAXIMA)
   */
  bool IsEnabled(const AntennaProcessorConfig& cfg) const override {
    return cfg.scenario_mode == PostFftScenarioMode::ALL_REQUIRED ||
           cfg.scenario_mode == PostFftScenarioMode::ALL_MAXIMA;
  }

  /**
   * @brief Запустить AllMaximaPipelineROCm и записать .beams в result->all_maxima.
   * @param ctx Shared context: kBufMagnitudes, kBufSpectrum, n_ant, nFFT, sample_rate, maxima_limit.
   *   @test { values=["valid_backend"] }
   */
  void Execute(PipelineContext& ctx) override {
    auto am_result = ctx.all_maxima_pipeline->Execute(
        ctx.buf(kBufMagnitudes),
        ctx.buf(kBufSpectrum),
        ctx.cfg->n_ant,
        ctx.nFFT,
        ctx.cfg->sample_rate,
        antenna_fft::OutputDestination::CPU,
        1,      // search_start (skip DC)
        0,      // search_end (0 = nFFT/2)
        ctx.cfg->maxima_limit);

    ctx.result->all_maxima = std::move(am_result.beams);
  }
};

}  // namespace strategies

#endif  // ENABLE_ROCM
