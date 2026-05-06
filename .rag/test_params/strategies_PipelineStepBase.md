---
schema_version: 1
repo: strategies
class_fqn: strategies::PipelineStepBase
file: E:/DSP-GPU/strategies/include/strategies/i_pipeline_step.hpp
line: 34
brief: "Базовый класс для шагов обработки в потоке, предоставляющий доступ к контексту PipelineContext"
methods_total: 2
methods_with_doxygen: 2
ai_generated: true
human_verified: false
parser_version: 2
synonyms_ru: ['Базовый шаг потока', 'Контекстный шаг', 'Шаг обработки', 'Потоковый шаг']
synonyms_en: ['Pipeline Step Base', 'Context Step', 'Processing Step', 'Flow Step']
tags: ['GPU', 'Pipeline', 'Context', 'Initialization']
---

# `strategies::PipelineStepBase` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class PipelineStepBase`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__pipeline_step_base__class_overview__v1 -->

**ЧТО**: Базовый класс для шагов обработки в потоке, предоставляющий доступ к контексту PipelineContext

**ЗАЧЕМ**: Упрощает получение контекста PipelineContext вместо GpuContext, уменьшая дублирование кода

**КАК**: Использует lazy initialization для методов Initialize/Release, абстрагируясь от GpuContext

**Пример**:
```cpp
#include "strategies/i_pipeline_step.hpp"

using namespace strategies;

class MyStep : public PipelineStepBase {
public:
    void Initialize(GpuContext&) override {
        // Реализация инициализации
    }
    void Release() override {
        // Освобождение ресурсов
    }
};

int main() {
    MyStep step;
    step.Initialize(...);
    step.Release();
    return 0;
}
```

<!-- /rag-block -->

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

