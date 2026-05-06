---
schema_version: 1
repo: strategies
class_fqn: strategies::AntennaProcessor
file: E:/DSP-GPU/strategies/include/strategies/antenna_processor.hpp
line: 18
brief: "Определяет интерфейс для обработки сигналов с использованием GPU. Обеспечивает абстракцию для работы с антеннами в пайплайне."
methods_total: 3
methods_with_doxygen: 3
ai_generated: true
human_verified: false
parser_version: 2
synonyms_ru: ['AntennaProcessor', 'AntennaProcessor_v1', 'SignalProcessor', 'AntennaStrategy']
synonyms_en: ['AntennaProcessor', 'AntennaProcessor_v1', 'SignalProcessor', 'AntennaStrategy']
tags: ['GPU', 'Signal Processing', 'Strategy Pattern', 'Pipeline']
---

# `strategies::AntennaProcessor` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class AntennaProcessor`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__antenna_processor__class_overview__v1 -->

**ЧТО**: Определяет интерфейс для обработки сигналов с использованием GPU. Обеспечивает абстракцию для работы с антеннами в пайплайне.

**ЗАЧЕМ**: Разделяет логику обработки сигналов от конфигурации и устройства, позволяя поддерживать разные стратегии обработки и оптимизировать использование GPU.

**КАК**: Абстрактный интерфейс с чисто виртуальными методами. Поддерживает паттерн Strategy для разных алгоритмов обработки. Интегрируется с системой чекпоинтов и потоками данных.

**Пример**:
```cpp
#include "strategies/antenna_processor.hpp"

using namespace strategies;

int main() {
    auto processor = std::make_unique<AntennaProcessor_v1>();
    processor->config().set_chunk_size(65536);
    auto result = processor->process(d_S, d_W);
    int gpu_id = processor->gpu_id();
    return 0;
}
```

<!-- /rag-block -->

## Связанные секции из Doc/

- `strategies__ap_c3_component__s_1_002__v1` (s_1): ``` ┌──────────────────────────────────────────────────── strategies ─────────────────────────────────────────────────────┐ │                                                                           …
- `strategies__ap_seq__pipeline_data_flow_002__v1` (pipeline_data_flow): ```  UserApp       AntennaProcessor_v1  Stream0(DMA) Stream1(Stats) Stream2(Main)  Stream3(SPost)    Result     │                │                │             │               │              │        …
- `strategies__ap_c1_systemcontext__s_2_system_context_diagram_002__v1` (s_2_system_context_diagram): ```  ┌────────────────────────────────────────────────────────────────────────────────┐  │                         ПОЛЬЗОВАТЕЛИ                                           │  │                          …
- `strategies__ap_c3_component__s_7_002__v1` (s_7): ``` strategies/ ├── include/ │   ├── antenna_processor.hpp              # AntennaProcessor (abstract base class, без 'I') │   ├── antenna_processor_v1.hpp           # AntennaProcessor_v1 (concrete, ин…
- `strategies__ap_seq__seq_4_chunking_vram_002__v1` (seq_4_chunking_vram): ```  UserApp       AntennaProcessor_v1   Stream0   Stream1    Stream2   WelfordAccum     │                │               │          │          │            │     │ process(S,W)   │               │   …

## Public-методы (3)

## Method 1: `process`

**Сигнатура** (`antenna_processor.hpp:80`):
```cpp
virtual AntennaResult process(const void* d_S, const void* d_W) = 0
```

**Параметры**:
- `d_S` — `const void*` *(pointer)* *(void\*)*
- `d_W` — `const void*` *(pointer)* *(void\*)*

**Возвращает**: `AntennaResult`

**Doxygen-источник**:
```cpp
/**
   * @brief Запустить полный pipeline антенной обработки.
   * @param d_S Входной сигнал [n_ant × n_samples] complex<float> на GPU.
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   * @param d_W Матрица весов [n_ant × n_ant] complex<float> на GPU.
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   * @return Агрегированный результат: статистики, пики, MinMax, метрики производительности.
   *   @test_check result.success == true (для валидных d_S и d_W)
   */
```

## Method 2: `config`

**Сигнатура** (`antenna_processor.hpp:99`):
```cpp
virtual const AntennaProcessorConfig& config() const = 0
```

**Возвращает**: `AntennaProcessorConfig`

**Doxygen-источник**:
```cpp
/**
   * @brief Возвращает текущий конфиг pipeline'а (n_ant, n_samples, scenario_mode, ...).
   *
   * @return Const-ссылка на хранимый AntennaProcessorConfig.
   *   @test_check result.n_ant > 0 && result.n_samples > 0
   */
```

## Method 3: `gpu_id`

**Сигнатура** (`antenna_processor.hpp:106`):
```cpp
virtual int gpu_id() const = 0
```

**Возвращает**: `int`

**Doxygen-источник**:
```cpp
/**
   * @brief Возвращает идентификатор GPU, на котором работает процессор.
   *
   * @return GPU id (0..GetDeviceCount()-1).
   *   @test_check result >= 0
   */
```

