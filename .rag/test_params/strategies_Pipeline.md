---
schema_version: 1
repo: strategies
class_fqn: strategies::Pipeline
file: E:/DSP-GPU/strategies/include/strategies/pipeline.hpp
line: 36
brief: "Управляет потоками выполнения и шагами обработки сигналов в GPU-пайплайне"
methods_total: 2
methods_with_doxygen: 2
ai_generated: true
human_verified: false
parser_version: 2
synonyms_ru: ['Пайплайн', 'SignalProcessor', 'GPUProcessing', 'ParallelExecution']
synonyms_en: ['Pipeline', 'SignalProcessor', 'GPUProcessing', 'ParallelExecution']
tags: ['GPU', 'ParallelProcessing', 'SignalProcessing', 'C++']
---

# `strategies::Pipeline` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class Pipeline`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__pipeline__class_overview__v1 -->

**ЧТО**: Управляет потоками выполнения и шагами обработки сигналов в GPU-пайплайне

**ЗАЧЕМ**: Оптимизирует обработку сигналов на GPU с поддержкой параллелизма и синхронизации потоков

**КАК**: Использует HIP для синхронизации параллельных потоков, разделяет шаги на последовательные и параллельные группы, кэширует шаги для быстрого поиска

**Пример**:
```cpp
#include "strategies/pipeline.hpp"
using namespace strategies;

int main() {
  Pipeline pipeline;
  pipeline.AddStep("stats", std::make_unique<StatsStep>());
  pipeline.AddStep("gemm", std::make_unique<GEMMStep>());
  PipelineContext ctx;
  ctx.cfg->SetParam("save_input", true);
  pipeline.Execute(ctx);
  IPipelineStep* step = pipeline.FindStep("stats");
  return 0;
}
```

<!-- /rag-block -->

## Связанные секции из Doc/

- `strategies__ap_seq__pipeline_data_flow_002__v1` (pipeline_data_flow): ```  UserApp       AntennaProcessor_v1  Stream0(DMA) Stream1(Stats) Stream2(Main)  Stream3(SPost)    Result     │                │                │             │               │              │        …
- `strategies__ap_c2_container__s_1_container_diagram_002__v1` (s_1_container_diagram):   EXTERNAL GPU PRODUCER                       GPU VRAM (AMD 9070: 16 ГБ / MI100: 32 ГБ)   ──────────────────────                      ─────────────────────────────────────────────────────   ┌─────────…
- `strategies__farrow_pipeline__pipeline_data_flow_002__v1` (pipeline_data_flow): ``` ┌─────────────────────────────────────────────────────────────┐ │ ВХОД: scenario = ScenarioBuilder.build()                    │ │   scenario['S']        → S_raw [n_ant × n_samples] complex │ │   s…
- `strategies__farrow_pipeline__pipeline_data_flow_004__v1` (pipeline_data_flow): ``` ┌─────────────────────────────────────────────────────────────┐ │ ВХОД: scenario = ScenarioBuilder.build()                    │ │   scenario['S']        → S_raw [n_ant × n_samples] complex │ │   s…
- `strategies__ap_c1_systemcontext__s_2_system_context_diagram_002__v1` (s_2_system_context_diagram): ```  ┌────────────────────────────────────────────────────────────────────────────────┐  │                         ПОЛЬЗОВАТЕЛИ                                           │  │                          …

## Public-методы (2)

## Method 1: `Execute`

**Сигнатура** (`pipeline.hpp:104`):
```cpp
void Execute(PipelineContext& ctx) { for (auto& entry : entries_) { if (entry.type == Entry::SEQUENTIAL) { if (entry.step->IsEnabled(*ctx.cfg)) { entry.step->Execute(ctx); } } else { auto& group = parallel_groups_[entry.parallel_group_index]; for (size_t i = 0; i < group.steps.size(); ++i) { if (group.steps[i]->IsEnabled(*ctx.cfg)) { group.steps[i]->Execute(ctx); } } // Sync all parallel streams for (auto stream : group.streams) { hipStreamSynchronize(stream); } } }
```

**Параметры**:
- `ctx` — `PipelineContext&`

**Doxygen-источник**:
```cpp
/**
   * @brief Выполнить все шаги в порядке entries_, скипая отключённые (IsEnabled==false).
   * @param ctx Shared context: kernels, streams, buffers, result.
   *   @test { values=["valid_backend"] }
   *
   * Для PARALLEL-группы все шаги запускаются последовательно (они стоят на разных
   * streams и параллелятся на GPU), затем hipStreamSynchronize по всем streams
   * группы — гарантия что следующий шаг увидит результаты.
   */
```

## Method 2: `FindStep`

**Сигнатура** (`pipeline.hpp:134`):
```cpp
IPipelineStep* FindStep(const char* name) const { for (auto& s : all_steps_) { if (std::strcmp(s->Name(), name) == 0) return s.get(); } return nullptr;
```

**Параметры**:
- `name` — `const char*` *(pointer)*

**Возвращает**: `IPipelineStep`

**Doxygen-источник**:
```cpp
/**
   * @brief Найти шаг по Name() (линейный O(N) поиск).
   * @param name Имя шага (как возвращает IPipelineStep::Name()).
   * @return Указатель на шаг или nullptr если не найден.
   *
   * Используется в тестах (например, AntennaProcessorTest) для прямого
   * вызова конкретного шага без перестройки pipeline'а.
   *   @test_check (result == nullptr) || (std::strcmp(result->Name(), name) == 0)
   */
```

