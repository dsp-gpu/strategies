---
schema_version: 1
repo: strategies
class_fqn: dsp::strategies::AntennaProcessor_v1
file: E:/DSP-GPU/strategies/include/strategies/antenna_processor_v1.hpp
line: 50
brief: "Обрабатывает сигналы с использованием GPU-потоков и событий для параллельной обработки данных."
methods_total: 3
methods_with_doxygen: 3
ai_generated: true
human_verified: false
parser_version: 2
synonyms_ru: ['Обработка антенн', 'GPU-обработка сигналов', 'Стратегия обработки', 'Потоковая обработка']
synonyms_en: ['Antenna Processing', 'GPU Signal Processing', 'Strategy Pattern', 'Stream Processing']
tags: ['GPU', 'HIP', 'Parallel Processing', 'Antennas', 'Strategies']
---

# `dsp::strategies::AntennaProcessor_v1` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class AntennaProcessor_v1`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__antenna_processor_v1__class_overview__v1 -->

**ЧТО**: Обрабатывает сигналы с использованием GPU-потоков и событий для параллельной обработки данных.

**ЗАЧЕМ**: Решает проблему последовательной обработки сигналов на CPU, обеспечивая высокую пропускную способность через асинхронные операции копирования и вычисления.

**КАК**: Использует HIP-потоки для асинхронной обработки, события для синхронизации, кэширование конфигурации, поддержку chunked VRAM-копирований и интеграцию с WelfordAccum для статистики.

**Пример**:
```cpp
#include "dsp/strategies/antenna_processor_v1.hpp"
using namespace dsp::strategies;

int main() {
    AntennaProcessor_v1 processor;
    void* d_S = ...;
    void* d_W = ...;
    auto result = processor.process(d_S, d_W);
    std::cout << "GPU ID: " << processor.gpu_id() << std::endl;
    return 0;
}
```

<!-- /rag-block -->

## Связанные секции из Doc/

- `strategies__ap_c3_component__s_1_002__v1` (s_1): ``` ┌──────────────────────────────────────────────────── strategies ─────────────────────────────────────────────────────┐ │                                                                           …
- `strategies__ap_seq__pipeline_data_flow_002__v1` (pipeline_data_flow): ```  UserApp       AntennaProcessor_v1  Stream0(DMA) Stream1(Stats) Stream2(Main)  Stream3(SPost)    Result     │                │                │             │               │              │        …
- `strategies__ap_c3_component__s_7_002__v1` (s_7): ``` strategies/ ├── include/ │   ├── antenna_processor.hpp              # AntennaProcessor (abstract base class, без 'I') │   ├── antenna_processor_v1.hpp           # AntennaProcessor_v1 (concrete, ин…
- `strategies__ap_seq__seq_4_chunking_vram_002__v1` (seq_4_chunking_vram): ```  UserApp       AntennaProcessor_v1   Stream0   Stream1    Stream2   WelfordAccum     │                │               │          │          │            │     │ process(S,W)   │               │   …
- `strategies__ap_c2_container__s_5_plantuml_container_diagram_001__v1` (s_5_plantuml_container_diagram): ## 5. PlantUML: Container Diagram  ```plantuml @startuml AP_C2_Container !include <C4/C4_Container> LAYOUT_WITH_LEGEND() title AntennaProcessor — C2: Container Diagram  Person(user, "C++ Engineer / Py…

## Public-методы (3)

## Method 1: `process`

**Сигнатура** (`antenna_processor_v1.hpp:126`):
```cpp
AntennaResult process(const void* d_S, const void* d_W) override
```

**Параметры**:
- `d_S` — `const void*` *(pointer)* *(void\*)*
- `d_W` — `const void*` *(pointer)* *(void\*)*

**Возвращает**: `AntennaResult`

**Doxygen-источник**:
```cpp
/**
   * @brief Запускает полный pipeline (Pipeline::Execute) на входе d_S/d_W; возвращает агрегированный результат.
   *
   * @param d_S Входной сигнал [n_ant × n_samples] complex<float> на GPU.
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   * @param d_W Матрица весов [n_ant × n_ant] complex<float> на GPU.
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   *
   * @return Результат: статистики, пики (по сценарию), MinMax, метрики производительности.
   *   @test_check result.success == true
   */
```

## Method 2: `config`

**Сигнатура** (`antenna_processor_v1.hpp:140`):
```cpp
const AntennaProcessorConfig& config() const override { return cfg_;
```

**Возвращает**: `AntennaProcessorConfig`

**Doxygen-источник**:
```cpp
/**
   * @brief Возвращает текущий конфиг pipeline'а (read-only).
   *
   * @return Const-ссылка на хранимый AntennaProcessorConfig.
   *   @test_check result.n_ant > 0 && result.n_samples > 0
   */
```

## Method 3: `gpu_id`

**Сигнатура** (`antenna_processor_v1.hpp:147`):
```cpp
int gpu_id() const override
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

