#pragma once

// ============================================================================
// IPipelineStep — контракт шага DSP-пайплайна (Ref03-C, Layer 6)
//
// ЧТО:    Узкий интерфейс шага композиционного pipeline'а: расширяет
//         drv_gpu_lib::IGpuOperation двумя методами — Execute(PipelineContext&)
//         и IsEnabled(AntennaProcessorConfig&). Конкретные шаги
//         (GemmStep, WindowFftStep, OneMaxStep, AllMaximaStep, MinMaxStep,
//         DebugStatsStep) наследуются от удобного PipelineStepBase, который
//         даёт пустые default-реализации IGpuOperation для тех шагов, что не
//         работают с GpuContext напрямую.
//
// ЗАЧЕМ:  Pipeline хранит std::unique_ptr<IPipelineStep> и единообразно
//         выполняет цепочку (Pipeline::Execute → for entry : entries_).
//         Без интерфейса Pipeline знал бы конкретные типы шагов (нарушение
//         DIP). Через IPipelineStep* — pluggable шаги, добавление нового
//         шага = только новый класс, Pipeline и Builder не меняются (OCP).
//         IsEnabled(cfg) — runtime-фильтр: один и тот же DAG обслуживает
//         разные сценарии (ALL_REQUIRED, ONE_MAX_PARABOLA, GLOBAL_MINMAX).
//
// ПОЧЕМУ: - ISP: Execute(PipelineContext&) намеренно ОТДЕЛЬНО от
//           IGpuOperation::Execute(GpuContext&) — у пайплайна другая модель
//           (shared context + результат + per-call inputs), нельзя сливать.
//         - PipelineStepBase даёт «нулевые» Initialize/IsReady/Release,
//           чтобы 95% шагов не дублировали бойлерплейт. Шаги, которые сами
//           держат ресурсы (например, kernel handle), могут переопределить.
//         - IsEnabled — host-side dispatch БЕЗ ветвлений в Execute: если
//           шаг отключён, он не вызывается (Pipeline::Execute проверяет ДО
//           вызова). Логика «надо/не надо» собрана в одном методе.
//         - Шаги получают ВСЁ из PipelineContext: backend, gpu_ctx, plan,
//           streams, events, buffers, sub-processors, results. GpuContext
//           напрямую не нужен → low coupling с core.
//
// Использование:
//   class MyStep : public PipelineStepBase {
//     const char* Name() const override { return "MyStep"; }
//     bool IsEnabled(const AntennaProcessorConfig& cfg) const override {
//       return cfg.scenario_mode == PostFftScenarioMode::ALL_REQUIRED;
//     }
//     void Execute(PipelineContext& ctx) override {
//       // launch kernel via ctx.gpu_ctx->GetKernel(...)
//       // record event ctx.event_*
//     }
//   };
//
// История:
//   - Создан: 2026-03-14 (Ref03-C, разделение IGpuOperation и IPipelineStep)
// ============================================================================

#if ENABLE_ROCM

#include <core/interface/i_gpu_operation.hpp>

namespace strategies {

struct PipelineContext;
struct AntennaProcessorConfig;

/**
 * @class IPipelineStep
 * @brief Интерфейс шага DSP-pipeline'а — Execute(ctx) + IsEnabled(cfg).
 *
 * @note Расширяет drv_gpu_lib::IGpuOperation; не инстанцируется.
 * @note Execute(PipelineContext&) — НЕ путать с IGpuOperation::Execute.
 * @see PipelineStepBase   — удобная база с пустыми Initialize/IsReady/Release.
 * @see Pipeline           — runner, владеет std::unique_ptr<IPipelineStep>.
 * @see PipelineBuilder    — fluent сборщик пайплайна.
 */
class IPipelineStep : public drv_gpu_lib::IGpuOperation {
public:
  /// Выполнить шаг на shared-контексте (kernels/streams/buffers/result).
  virtual void Execute(PipelineContext& ctx) = 0;

  /// true — шаг должен выполниться при текущем config (sceanrio_mode и т.д.).
  virtual bool IsEnabled(const AntennaProcessorConfig& cfg) const = 0;
};

/**
 * @class PipelineStepBase
 * @brief Удобная база: пустые Initialize/IsReady/Release для шагов без своих GPU-ресурсов.
 *
 * @note Большинство шагов берут ресурсы из PipelineContext, не из GpuContext.
 *       Если шаг сам держит kernel handle — переопределить Initialize/Release.
 */
class PipelineStepBase : public IPipelineStep {
public:
  // IGpuOperation defaults (pipeline steps don't use GpuContext directly)
  void Initialize(drv_gpu_lib::GpuContext&) override {}
  bool IsReady() const override { return true; }
  void Release() override {}
};

}  // namespace strategies

#endif  // ENABLE_ROCM
