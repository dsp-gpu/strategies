#pragma once

// ============================================================================
// PipelineBuilder — fluent-конструктор Pipeline (Ref03-C, GoF Builder)
//
// ЧТО:    Промежуточный объект для декларативной сборки Pipeline. Имеет
//         три метода-добавителя: add(step), add_if(cond, step),
//         add_parallel(group, streams) — каждый возвращает *this для
//         цепочки. После build() возвращает std::unique_ptr<Pipeline> и
//         перекладывает владение шагами через std::move.
//
// ЗАЧЕМ:  Без Builder сборка Pipeline в фасаде выглядела бы как 30 строк
//         push_back в три разных vector'а с ручным заполнением Entry.
//         Builder инкапсулирует эту бухгалтерию: один вызов add() ставит
//         запись в entries_ + кладёт unique_ptr в steps_, один вызов
//         add_parallel() — Entry типа PARALLEL + ParallelGroup в
//         parallel_groups_. Фасад читается как декларация: «GEMM, потом
//         FFT, потом параллельно три post-FFT».
//
// ПОЧЕMУ: - GoF Builder: build() — финализация, после неё Builder
//           разрушается (move-out внутренних vector'ов). Повторный build
//           даст пустой Pipeline (намеренно — единичная сборка).
//         - add_if(cond, step) — конструктор-conditional: если condition
//           false, шаг ВООБЩЕ не создаётся в Pipeline (отличие от
//           IsEnabled, который создаёт шаг и решает в runtime).
//           Для редко используемых тяжёлых шагов это экономит память.
//         - add_parallel принимает ВСЕ шаги группы + ВСЕ streams сразу —
//           гарантирует, что они попадут в один Entry::PARALLEL и не
//           разъедутся по индексам.
//         - friend class Pipeline (см. Pipeline::PipelineBuilder
//           в pipeline.hpp) — Builder лезет в приватные all_steps_ /
//           entries_ / parallel_groups_, чтобы избежать публичных
//           setter'ов которые порушат immutability Pipeline.
//
// Использование:
//   auto pipe = PipelineBuilder()
//     .add(std::make_unique<GemmStep>())
//     .add(std::make_unique<WindowFftStep>())
//     .add_if(cfg.scenario_mode != PostFftScenarioMode::NONE,
//             std::make_unique<DebugStatsStep>(DebugPoint::POST_FFT))
//     .add_parallel(std::move(post_steps),
//                   {ctx.stream_post_a, ctx.stream_post_b, ctx.stream_post_c})
//     .build();
//
// История:
//   - Создан: 2026-03-14 (Ref03-C, GoF Builder для Pipeline)
// ============================================================================

#if ENABLE_ROCM

#include <strategies/pipeline.hpp>

#include <hip/hip_runtime.h>
#include <memory>
#include <vector>

namespace strategies {

/**
 * @class PipelineBuilder
 * @brief Fluent Builder для Pipeline: add()/add_if()/add_parallel() → build().
 *
 * @note Move-only по сути: build() переносит владение шагами в Pipeline.
 * @note Цепочка вызовов возвращает *this — пишется в одну C++-выражение.
 * @see Pipeline           — финальный объект, immutable после build().
 * @see IPipelineStep      — контракт шага, передаётся через unique_ptr.
 */
class PipelineBuilder {
public:
  /// Добавить sequential-шаг (выполняется по очереди после предыдущих).
  PipelineBuilder& add(std::unique_ptr<IPipelineStep> step) {
    Pipeline::Entry entry;
    entry.type = Pipeline::Entry::SEQUENTIAL;
    entry.step = step.get();
    entries_.push_back(entry);
    steps_.push_back(std::move(step));
    return *this;
  }

  /// Добавить шаг ТОЛЬКО если condition==true (compile-time/host-side фильтр).
  /// Отличие от IPipelineStep::IsEnabled: при condition==false шаг даже
  /// не создаётся в Pipeline → экономия памяти на отключённых тяжёлых шагах.
  PipelineBuilder& add_if(bool condition, std::unique_ptr<IPipelineStep> step) {
    if (condition) return add(std::move(step));
    return *this;
  }

  /// Добавить parallel-группу: шаги [i] запускаются на streams[i] параллельно.
  /// Pipeline::Execute сделает hipStreamSynchronize по ВСЕМ streams группы
  /// перед переходом к следующему Entry — синхронизация автоматическая.
  PipelineBuilder& add_parallel(
      std::vector<std::unique_ptr<IPipelineStep>> group_steps,
      std::vector<hipStream_t> streams) {
    ParallelGroup pg;
    Pipeline::Entry entry;
    entry.type = Pipeline::Entry::PARALLEL;
    entry.parallel_group_index = parallel_groups_.size();

    for (size_t i = 0; i < group_steps.size(); ++i) {
      pg.steps.push_back(group_steps[i].get());
      if (i < streams.size()) pg.streams.push_back(streams[i]);
    }

    entries_.push_back(entry);
    for (auto& s : group_steps) steps_.push_back(std::move(s));
    parallel_groups_.push_back(std::move(pg));
    return *this;
  }

  /// Финализация: создать Pipeline и перенести в него всё накопленное.
  /// После build() Builder можно разрушить (внутренние vector'ы пусты).
  std::unique_ptr<Pipeline> build() {
    auto p = std::make_unique<Pipeline>();
    p->all_steps_ = std::move(steps_);
    p->entries_ = std::move(entries_);
    p->parallel_groups_ = std::move(parallel_groups_);
    return p;
  }

private:
  std::vector<std::unique_ptr<IPipelineStep>> steps_;
  std::vector<Pipeline::Entry> entries_;
  std::vector<ParallelGroup> parallel_groups_;
};

}  // namespace strategies

#endif  // ENABLE_ROCM
