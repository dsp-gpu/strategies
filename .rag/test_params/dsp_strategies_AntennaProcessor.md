---
schema_version: 1
repo: strategies
class_fqn: dsp::strategies::AntennaProcessor
file: /home/alex/DSP-GPU/strategies/include/dsp/strategies/antenna_processor.hpp
line: 67
brief: "/**  * @class AntennaProcessor  * @brief Layer 6 Ref03 фасад: pure-virtual интерфейс pipeline'а антенной обработки.  *  * @note Pure interface — нельзя инстанцировать. Реализации: AntennaProcessor_v1,"
methods_total: 3
methods_with_doxygen: 3
ai_generated: false
human_verified: false
parser_version: 1
---

# `dsp::strategies::AntennaProcessor` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class AntennaProcessor`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__antenna_processor__class_overview__v1 -->

/**
 * @class AntennaProcessor
 * @brief Layer 6 Ref03 фасад: pure-virtual интерфейс pipeline'а антенной обработки.
 *
 * @note Pure interface — нельзя инстанцировать. Реализации: AntennaProcessor_v1, AntennaProcessorTest.
 * @note Не thread-safe. Один экземпляр = один владелец GPU-ресурсов pipeline'а.
 * @see AntennaProcessor_v1 — ROCm-реализация (concrete Strategy)
 * @see AntennaProcessorTest — step-by-step расширение для отладки и Python-валидации
 * @see AntennaProcessorConfig — POD-конфиг pipeline'а
 * @see PostFftScenarioMode — селектор post-FFT сценариев
 */

<!-- /rag-block -->

## Связанные секции из Doc/

- `strategies__ap_c3_component__s_1_002__v1` (s_1): ``` ┌──────────────────────────────────────────────────── strategies ─────────────────────────────────────────────────────┐ │                                                                           …
- `strategies__ap_seq__pipeline_data_flow_002__v1` (pipeline_data_flow): ```  UserApp       AntennaProcessor_v1  Stream0(DMA) Stream1(Stats) Stream2(Main)  Stream3(SPost)    Result     │                │                │             │               │              │        …
- `strategies__ap_c1_systemcontext__s_2_system_context_diagram_002__v1` (s_2_system_context_diagram): ```  ┌────────────────────────────────────────────────────────────────────────────────┐  │                         ПОЛЬЗОВАТЕЛИ                                           │  │                          …
- `strategies__patterns__strategy_002__v1` (strategy): - **`dsp::strategies::DebugStatsStep`** — `strategies/include/strategies/steps/debug_stats_step.hpp:30`   - Параметризованный pipeline-шаг с тремя инстансами по точке наблюдения (DebugPoint::PRE_INPUT…
- `strategies__ap_c3_component__s_7_002__v1` (s_7): ``` strategies/ ├── include/ │   ├── antenna_processor.hpp              # AntennaProcessor (abstract base class, без 'I') │   ├── antenna_processor_v1.hpp           # AntennaProcessor_v1 (concrete, ин…

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

