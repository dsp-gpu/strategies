---
schema_version: 1
repo: strategies
class_fqn: strategies::WeightGenerator
file: E:/DSP-GPU/strategies/include/strategies/weight_generator.hpp
line: 29
brief: "Генерирует и загружает на GPU матрицу весов для алгоритма delay-and-sum."
methods_total: 2
methods_with_doxygen: 2
ai_generated: true
human_verified: false
parser_version: 2
synonyms_ru: ['Генератор Весов', 'Матрица Весов', 'WeightMatrix', 'DelayAndSumMatrix']
synonyms_en: ['WeightGenerator', 'WeightMatrix', 'DelayAndSumMatrix', 'BeamformingWeights']
tags: ['GPU', 'радиолокация', 'delay-and-sum', 'ROCm', 'HIP']
---

# `strategies::WeightGenerator` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class WeightGenerator`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__weight_generator__class_overview__v1 -->

**ЧТО**: Генерирует и загружает на GPU матрицу весов для алгоритма delay-and-sum.

**ЗАЧЕМ**: Разделяет вычисления на CPU и загрузку на GPU для оптимизации памяти и производительности.

**КАК**: Статический класс с разделением логики генерации и загрузки. Использует lazy init для отложенной инициализации весов.

**Пример**:
```cpp
#include <strategies/antenna_processor_v1.hpp>
#include "weight_generator.hpp"

strategies::AntennaProcessorConfig cfg;
cfg.n_ant = 5;

strategies::WeightParams wp;
wp.n_ant = 5; wp.f0 = 2e6;
auto W_cpu = strategies::WeightGenerator::generate_delay_and_sum(wp);
void* d_W = strategies,WeightGenerator::upload_to_gpu(backend, W_cpu);
```

<!-- /rag-block -->

## Связанные секции из Doc/

- `strategies__gpu__s_8_001__v1` (s_8): ## 8. Файловая структура  ``` strategies/ ├── CMakeLists.txt ├── include/ │   ├── antenna_processor.hpp          # AntennaProcessor (abstract) │   ├── antenna_processor_v1.hpp       # AntennaProcessor…
- `strategies__gpu__api_003__v1` (api):   // Пошаговый API (порядок важен!)   void step_0_prepare_input(const void* d_S, const void* d_W);   void step_1_debug_input();       // D2H stats on d_S → CPU   void step_2_gemm();              // X …
- `strategies__api__pipeline_data_flow_002__v1` (pipeline_data_flow): ### Production — полный pipeline  ```cpp #include <strategies/antenna_processor_v1.hpp> #include "weight_generator.hpp"  // ── 1. Конфиг ────────────────────────────────────────────────────────── stra…
- `strategies__api__s_4_weightgenerator__v1` (s_4_weightgenerator): ## 4. WeightGenerator  **Файл**: `strategies/include/weight_generator.hpp` **Реализация**: `strategies/src/weight_generator.cpp`  Статический класс. Генерация и загрузка матрицы весов.  ```cpp namespa…
- `strategies__quick__c__v1` (c): ## Быстрый старт — C++  ```cpp #include <strategies/antenna_processor_v1.hpp> #include "weight_generator.hpp"  // 1. Конфиг strategies::AntennaProcessorConfig cfg; cfg.n_ant          = 5; cfg.n_sample…

## Public-методы (2)

## Method 1: `generate_delay_and_sum`

**Сигнатура** (`weight_generator.hpp:92`):
```cpp
static std::vector<std::complex<float>> generate_delay_and_sum( const WeightParams& params)
```

**Параметры**:
- `params` — `const WeightParams&`

**Возвращает**: `std::vector<std::complex<float>>`

**Doxygen-источник**:
```cpp
/**
   * @brief Сгенерировать матрицу весов Delay-and-Sum.
   * @param params Количество антенн, частота, базовая задержка и шаг.
   *   @test_ref WeightParams
   * @return Flat row-major матрица [n_ant × n_ant] complex<float>.
   *
   * W[beam][ant] = exp(-j·2π·f0·τ[ant]) / √N_ant, τ[ant] = tau_base + ant·tau_step.
   *   @test_check result.size() == params.n_ant * params.n_ant
   */
```

## Method 2: `upload_to_gpu`

**Сигнатура** (`weight_generator.hpp:102`):
```cpp
static void* upload_to_gpu( void* backend, // drv_gpu_lib::IBackend* const std::vector<std::complex<float>>& weights)
```

**Параметры**:
- `backend` — `void*` *(pointer)* *(void\*)*
- `weights` — `const std::vector<std::complex<float>>&`

**Doxygen-источник**:
```cpp
/**
   * @brief Залить матрицу весов на GPU через IBackend.
   * @param backend Указатель на drv_gpu_lib::IBackend (передаётся как void*).
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   * @param weights Flat [n_ant × n_ant] complex<float> матрица.
   * @return Device-pointer (caller обязан освободить через backend->Free()).
   */
```

