---
schema_version: 1
repo: strategies
class_fqn: dsp::strategies::PipelineBuilder
file: /home/alex/DSP-GPU/strategies/include/dsp/strategies/pipeline_builder.hpp
line: 68
brief: "/**  * @class PipelineBuilder  * @brief Fluent Builder для Pipeline: add()/add_if()/add_parallel() → build().  *  * @note Move-only по сути: build() переносит владение шагами в Pipeline.  * @note Цепо"
methods_total: 4
methods_with_doxygen: 4
ai_generated: false
human_verified: false
parser_version: 1
---

# `dsp::strategies::PipelineBuilder` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class PipelineBuilder`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__pipeline_builder__class_overview__v1 -->

/**
 * @class PipelineBuilder
 * @brief Fluent Builder для Pipeline: add()/add_if()/add_parallel() → build().
 *
 * @note Move-only по сути: build() переносит владение шагами в Pipeline.
 * @note Цепочка вызовов возвращает *this — пишется в одну C++-выражение.
 * @see Pipeline           — финальный объект, immutable после build().
 * @see IPipelineStep      — контракт шага, передаётся через unique_ptr.
 */

<!-- /rag-block -->

## Связанные секции из Doc/

- `strategies__meta__claude_card__v1` (meta_claude): <!-- type:meta_claude repo:strategies source:strategies/CLAUDE.md -->  # strategies — Repository Card  _Источник: `strategies/CLAUDE.md`_  # 🤖 CLAUDE — `strategies`  > Композиционные стратегии: `IPipe…
- `strategies__pipeline__class_overview__v1` (class_overview): /**  * @class Pipeline  * @brief Runner упорядоченной цепочки IPipelineStep (sequential + parallel groups).  *  * @note Immutable после build() — структура entries_ не меняется.  * @note Execute(ctx) …
- `strategies__patterns__builder__v1` (builder): ## Builder  > Поэтапное конструирование сложного объекта.   - **`dsp::strategies::PipelineBuilder`** — `strategies/include/strategies/pipeline_builder.hpp:28`   - Промежуточный объект для декларативно…

## Public-методы (4)

## Method 1: `add`

**Сигнатура** (`pipeline_builder.hpp:78`):
```cpp
PipelineBuilder& add(std::unique_ptr<IPipelineStep> step) { Pipeline::Entry entry; entry.type = Pipeline::Entry::SEQUENTIAL; entry.step = step.get(); entries_.push_back(entry); steps_.push_back(std::move(step)); return *this;
```

**Параметры**:
- `step` — `std::unique_ptr<IPipelineStep>`

**Возвращает**: `PipelineBuilder`

**Doxygen-источник**:
```cpp
/**
   * @brief Добавляет sequential-шаг (выполняется по очереди после предыдущих).
   *
   * @param step Уникальное владение IPipelineStep; builder перенимает.
   *
   * @return *this для fluent-цепочки.
   *   @test_check &result == this
   */
```

## Method 2: `add_if`

**Сигнатура** (`pipeline_builder.hpp:96`):
```cpp
PipelineBuilder& add_if(bool condition, std::unique_ptr<IPipelineStep> step) { if (condition) return add(std::move(step)); return *this;
```

**Параметры**:
- `condition` — `bool`
- `step` — `std::unique_ptr<IPipelineStep>`

**Возвращает**: `PipelineBuilder`

**Doxygen-источник**:
```cpp
/**
   * @brief Добавляет шаг ТОЛЬКО если condition==true; иначе шаг уничтожается до build().
   *
   * @param condition Compile-time/host-side фильтр (отличается от IsEnabled — здесь шаг даже не создаётся в Pipeline).
   * @param step Уникальное владение IPipelineStep.
   *
   * @return *this для fluent-цепочки.
   *   @test_check &result == this
   */
```

## Method 3: `add_parallel`

**Сигнатура** (`pipeline_builder.hpp:110`):
```cpp
PipelineBuilder& add_parallel( std::vector<std::unique_ptr<IPipelineStep>> group_steps, std::vector<hipStream_t> streams) { ParallelGroup pg; Pipeline::Entry entry; entry.type = Pipeline::Entry::PARALLEL; entry.parallel_group_index = parallel_groups_.size(); for (size_t i = 0; i < group_steps.size(); ++i) { pg.steps.push_back(group_steps[i].get()); if (i < streams.size()) pg.streams.push_back(streams[i]); } entries_.push_back(entry); for (auto& s : group_steps) steps_.push_back(std::move(s)); parallel_groups_.push_back(std::move(pg)); return *this;
```

**Параметры**:
- `group_steps` — `std::vector<std::unique_ptr<IPipelineStep>>`
- `streams` — `std::vector<hipStream_t>`

**Возвращает**: `PipelineBuilder`

**Doxygen-источник**:
```cpp
/**
   * @brief Добавляет parallel-группу: шаги [i] запускаются на streams[i] параллельно. Sync — после группы.
   *
   * @param group_steps Шаги группы; запускаются параллельно на разных stream'ах.
   * @param streams Соответствующие streams (по одному на шаг); Pipeline::Execute синхронизирует все после.
   *
   * @return *this для fluent-цепочки.
   *   @test_check &result == this
   */
```

## Method 4: `build`

**Сигнатура** (`pipeline_builder.hpp:135`):
```cpp
std::unique_ptr<Pipeline> build() { auto p = std::make_unique<Pipeline>(); p->all_steps_ = std::move(steps_); p->entries_ = std::move(entries_); p->parallel_groups_ = std::move(parallel_groups_); return p;
```

**Возвращает**: `std::unique_ptr<Pipeline>`

**Doxygen-источник**:
```cpp
/**
   * @brief Финализирует builder и создаёт Pipeline (immutable). После — builder можно разрушить.
   *
   * @return unique_ptr<Pipeline> с перемещённым all_steps_/entries_/parallel_groups_.
   *   @test_check result != nullptr
   */
```

