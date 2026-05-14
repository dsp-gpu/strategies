---
schema_version: 1
repo: strategies
class_fqn: dsp::strategies::PipelineStepBase
file: /home/alex/DSP-GPU/strategies/include/dsp/strategies/i_pipeline_step.hpp
line: 99
brief: "/**  * @class PipelineStepBase  * @brief Удобная база: пустые Initialize/IsReady/Release для шагов без своих GPU-ресурсов.  *  * @note Большинство шагов берут ресурсы из PipelineContext, не из GpuCont"
methods_total: 2
methods_with_doxygen: 2
ai_generated: false
human_verified: false
parser_version: 1
---

# `dsp::strategies::PipelineStepBase` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class PipelineStepBase`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__pipeline_step_base__class_overview__v1 -->

/**
 * @class PipelineStepBase
 * @brief Удобная база: пустые Initialize/IsReady/Release для шагов без своих GPU-ресурсов.
 *
 * @note Большинство шагов берут ресурсы из PipelineContext, не из GpuContext.
 *       Если шаг сам держит kernel handle — переопределить Initialize/Release.
 */

<!-- /rag-block -->

## Связанные секции из Doc/

- `strategies__meta__claude_card__v1` (meta_claude): <!-- type:meta_claude repo:strategies source:strategies/CLAUDE.md -->  # strategies — Repository Card  _Источник: `strategies/CLAUDE.md`_  # 🤖 CLAUDE — `strategies`  > Композиционные стратегии: `IPipe…
- `strategies__patterns__strategy_002__v1` (strategy): - **`dsp::strategies::DebugStatsStep`** — `strategies/include/strategies/steps/debug_stats_step.hpp:30`   - Параметризованный pipeline-шаг с тремя инстансами по точке наблюдения (DebugPoint::PRE_INPUT…

## Public-методы (2)

## Method 1: `Initialize`

**Сигнатура** (`i_pipeline_step.hpp:106`):
```cpp
void Initialize(drv_gpu_lib::GpuContext&) override
```

**Параметры**:
- `_unnamed_` — `drv_gpu_lib::GpuContext&`

**Doxygen-источник**:
```cpp
/**
   * @brief No-op: pipeline-шаги берут ресурсы из PipelineContext, а не из GpuContext.
   *
   */
```

## Method 2: `Release`

**Сигнатура** (`i_pipeline_step.hpp:111`):
```cpp
void Release() override
```

**Doxygen-источник**:
```cpp
/**
   * @brief No-op: ресурсы pipeline-шагов освобождаются вместе с PipelineContext.
   */
```

