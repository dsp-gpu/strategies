#pragma once

// ============================================================================
// Pipeline — runner упорядоченной цепочки IPipelineStep (Ref03-C, Layer 6)
//
// ЧТО:    Owning-контейнер шагов + исполнитель. Хранит
//         std::vector<std::unique_ptr<IPipelineStep>> all_steps_ (владение)
//         и vector<Entry> entries_ (порядок исполнения). Каждый Entry —
//         либо SEQUENTIAL (один шаг), либо PARALLEL (группа шагов,
//         запускаемых на разных hipStream и синхронизируемых в конце).
//         Execute(ctx) проходит entries_ по порядку, для каждого
//         SEQUENTIAL вызывает step->Execute(ctx) если IsEnabled(cfg)==true,
//         для PARALLEL — последовательно запускает все шаги (они уже на
//         своих streams, hipStreamSynchronize в конце группы).
//         FindStep(name) — линейный поиск по Name() для тестов.
//
// ЗАЧЕМ:  AntennaProcessor (фасад) хранит готовый Pipeline и в process()
//         делает один вызов Execute(ctx). Логика «какие шаги в каком
//         порядке + где параллелить» собрана в одном объекте, а не
//         размазана по фасаду. Тесты могут дёргать конкретный шаг через
//         FindStep("OneMaxParabola") без перестройки всего pipeline'а.
//
// ПОЧЕМУ: - Immutable after build: Pipeline создаётся через
//           PipelineBuilder::build() (friend access к приватным членам),
//           дальше структура не меняется → thread-safe для Execute().
//         - PARALLEL-группа: шаги ставятся на свои streams внутри
//           Execute, после группы делается hipStreamSynchronize по ВСЕМ
//           streams группы — гарантия что следующий SEQUENTIAL шаг
//           видит результаты всех параллельных.
//         - IsEnabled проверяется ВНЕ шага (в Pipeline::Execute) —
//           отключённый шаг не вызывается, никакого раннего exit внутри.
//         - FindStep линейный (O(N)) сознательно: шагов десятки, hash-map
//           overkill, плюс тестам важна простота.
//
// Использование:
//   auto pipe = PipelineBuilder()
//     .add(std::make_unique<GemmStep>())
//     .add(std::make_unique<WindowFftStep>())
//     .add_parallel({...}, {s_a, s_b, s_c})
//     .build();
//   pipe->Execute(ctx);                            // штатный путь
//   auto* one_max = pipe->FindStep("OneMaxParabola");  // для теста
//
// История:
//   - Создан: 2026-03-14 (Ref03-C, выделено из AntennaProcessor)
// ============================================================================

#if ENABLE_ROCM

#include <strategies/i_pipeline_step.hpp>
#include <strategies/pipeline_context.hpp>

#include <hip/hip_runtime.h>

#include <vector>
#include <memory>
#include <string>
#include <cstring>

namespace strategies {

/**
 * @struct ParallelGroup
 * @brief Группа шагов на разных hipStream, синхронизируется после исполнения.
 * @note Указатели non-owning — владение шагами в Pipeline::all_steps_.
 */
struct ParallelGroup {
  std::vector<IPipelineStep*> steps;     ///< non-owning
  std::vector<hipStream_t>    streams;   ///< one per step
};

/**
 * @class Pipeline
 * @brief Runner упорядоченной цепочки IPipelineStep (sequential + parallel groups).
 *
 * @note Immutable после build() — структура entries_ не меняется.
 * @note Execute(ctx) проверяет IsEnabled(cfg) ДО вызова шага (отключённые скипаются).
 * @see PipelineBuilder — единственный способ построения (friend access).
 * @see IPipelineStep   — контракт шага.
 * @see PipelineContext — shared state, передаётся в Execute.
 */
class Pipeline {
public:
  /**
   * @struct Entry
   * @brief Один пункт в порядке исполнения: либо одиночный шаг, либо ссылка на parallel-группу.
   */
  struct Entry {
    enum Type { SEQUENTIAL, PARALLEL };
    Type type = SEQUENTIAL;
    IPipelineStep* step = nullptr;         ///< for SEQUENTIAL
    size_t parallel_group_index = 0;       ///< for PARALLEL
  };

  /**
   * @brief Выполнить все шаги в порядке entries_, скипая отключённые (IsEnabled==false).
   * @param ctx Shared context: kernels, streams, buffers, result.
   *   @test { values=["valid_backend"] }
   *
   * Для PARALLEL-группы все шаги запускаются последовательно (они стоят на разных
   * streams и параллелятся на GPU), затем hipStreamSynchronize по всем streams
   * группы — гарантия что следующий шаг увидит результаты.
   */
  void Execute(PipelineContext& ctx) {
    for (auto& entry : entries_) {
      if (entry.type == Entry::SEQUENTIAL) {
        if (entry.step->IsEnabled(*ctx.cfg)) {
          entry.step->Execute(ctx);
        }
      } else {
        auto& group = parallel_groups_[entry.parallel_group_index];
        for (size_t i = 0; i < group.steps.size(); ++i) {
          if (group.steps[i]->IsEnabled(*ctx.cfg)) {
            group.steps[i]->Execute(ctx);
          }
        }
        // Sync all parallel streams
        for (auto stream : group.streams) {
          hipStreamSynchronize(stream);
        }
      }
    }
  }

  /**
   * @brief Найти шаг по Name() (линейный O(N) поиск).
   * @param name Имя шага (как возвращает IPipelineStep::Name()).
   * @return Указатель на шаг или nullptr если не найден.
   *
   * Используется в тестах (например, AntennaProcessorTest) для прямого
   * вызова конкретного шага без перестройки pipeline'а.
   *   @test_check (result == nullptr) || (std::strcmp(result->Name(), name) == 0)
   */
  IPipelineStep* FindStep(const char* name) const {
    for (auto& s : all_steps_) {
      if (std::strcmp(s->Name(), name) == 0) return s.get();
    }
    return nullptr;
  }

private:
  friend class PipelineBuilder;

  std::vector<std::unique_ptr<IPipelineStep>> all_steps_;   ///< ownership
  std::vector<Entry> entries_;                               ///< execution order
  std::vector<ParallelGroup> parallel_groups_;
};

}  // namespace strategies

#endif  // ENABLE_ROCM
