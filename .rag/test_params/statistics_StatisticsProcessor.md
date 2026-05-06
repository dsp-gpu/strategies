---
schema_version: 1
repo: strategies
class_fqn: statistics::StatisticsProcessor
file: E:/DSP-GPU/strategies/include/strategies/pipeline_context.hpp
line: 30
brief: "Обрабатывает статистические характеристики сигналов в пайплайне обработки сигналов."
methods_total: 0
methods_with_doxygen: 0
ai_generated: true
human_verified: false
parser_version: 2
synonyms_ru: ['Обработчик статистики', 'Процессор статистики', 'Статистический анализатор', 'Анализатор данных']
synonyms_en: ['StatisticsHandler', 'StatisticalProcessor', 'DataAnalyzer', 'SignalStatsProcessor']
tags: ['GPU', 'статистика', 'ROCm', 'HIP', 'обработка сигналов', 'пайплайн']
---

# `statistics::StatisticsProcessor` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class StatisticsProcessor`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__statistics_processor__class_overview__v1 -->

**ЧТО**: Обрабатывает статистические характеристики сигналов в пайплайне обработки сигналов.

**ЗАЧЕМ**: Решает задачу эффективного вычисления моментов и сортировки данных на GPU с использованием алгоритмов Welford и radix sort.

**КАК**: Использует lazy init для отложенной инициализации, кэширование результатов, batch-обработку данных. Интегрируется с HIP/ROCm для GPU-вычислений.

**Пример**:
```cpp
#include "pipeline_context.hpp"
using namespace strategies;

int main() {
    auto stats = std::make_shared<StatisticsProcessor>();
    auto processor = std::make_shared<AntennaProcessor_v1>();
    processor->set_pre_stats(stats);
    processor->process(d_S, d_W);
    auto result = processor->config().stats();
    return 0;
}
```

<!-- /rag-block -->

## Связанные секции из Doc/

- `strategies__ap_c3_component__s_1_002__v1` (s_1): ``` ┌──────────────────────────────────────────────────── strategies ─────────────────────────────────────────────────────┐ │                                                                           …
- `strategies__ap_c1_systemcontext__s_2_system_context_diagram_002__v1` (s_2_system_context_diagram): ```  ┌────────────────────────────────────────────────────────────────────────────────┐  │                         ПОЛЬЗОВАТЕЛИ                                           │  │                          …
- `strategies__ap_c2_container__s_5_plantuml_container_diagram_001__v1` (s_5_plantuml_container_diagram): ## 5. PlantUML: Container Diagram  ```plantuml @startuml AP_C2_Container !include <C4/C4_Container> LAYOUT_WITH_LEGEND() title AntennaProcessor — C2: Container Diagram  Person(user, "C++ Engineer / Py…
- `strategies__ap_c4_code__s_9_plantuml_class_diagram_001__v1` (s_9_plantuml_class_diagram): ## 9. PlantUML: Class Diagram  ```plantuml @startuml AP_C4_ClassDiagram !theme C4_united from <C4/themes> title AntennaProcessor — C4: Code (Class Diagram)  interface AntennaProcessor {     + process(…
- `strategies__ap_c1_systemcontext__s_5_plantuml__v1` (s_5_plantuml): ## 5. PlantUML  ```plantuml @startuml AP_C1_SystemContext !include <C4/C4_Context> LAYOUT_WITH_LEGEND() title AntennaProcessor — C1: System Context  Person(cpp_eng, "C++ Engineer", "Интегрирует Antenn…

## Public-методы (0)

_У класса StatisticsProcessor не найдено public-методов в header'е._

