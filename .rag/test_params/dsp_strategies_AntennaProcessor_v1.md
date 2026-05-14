---
schema_version: 1
repo: strategies
class_fqn: dsp::strategies::AntennaProcessor_v1
file: /home/alex/DSP-GPU/strategies/include/dsp/strategies/antenna_processor_v1.hpp
line: 103
brief: "/**  * @class AntennaProcessor_v1  * @brief ROCm-реализация AntennaProcessor: GEMM + Window+FFT + post-FFT сценарии.  *  * @note Move/copy запрещены — owns hipBLAS handle + hipFFT plan + 7 streams + G"
methods_total: 3
methods_with_doxygen: 3
ai_generated: false
human_verified: false
parser_version: 1
---

# `dsp::strategies::AntennaProcessor_v1` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class AntennaProcessor_v1`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__antenna_processor_v1__class_overview__v1 -->

/**
 * @class AntennaProcessor_v1
 * @brief ROCm-реализация AntennaProcessor: GEMM + Window+FFT + post-FFT сценарии.
 *
 * @note Move/copy запрещены — owns hipBLAS handle + hipFFT plan + 7 streams + GPU buffers.
 * @note Требует #if ENABLE_ROCM. На non-ROCm сборках большая часть кода скрыта макросом.
 * @note Lifecycle: ctor(backend, cfg) → (опц.) set_external_weights → process / step_* → dtor.
 * @note Не thread-safe. Один экземпляр = один владелец GPU-ресурсов.
 * @see AntennaProcessor — родительский Strategy-интерфейс
 * @see AntennaProcessorTest — наследник со step-by-step API для тестов
 * @see ICheckpointSave — стратегия checkpoint-сохранения (default = NullCheckpointSave)
 * @ingroup grp_strategies
 */

<!-- /rag-block -->

## Связанные секции из Doc/

- `strategies__ap_c3_component__s_1_002__v1` (s_1): ``` ┌──────────────────────────────────────────────────── strategies ─────────────────────────────────────────────────────┐ │                                                                           …
- `strategies__ap_seq__pipeline_data_flow_002__v1` (pipeline_data_flow): ```  UserApp       AntennaProcessor_v1  Stream0(DMA) Stream1(Stats) Stream2(Main)  Stream3(SPost)    Result     │                │                │             │               │              │        …
- `strategies__ap_c3_component__s_7_002__v1` (s_7): ``` strategies/ ├── include/ │   ├── antenna_processor.hpp              # AntennaProcessor (abstract base class, без 'I') │   ├── antenna_processor_v1.hpp           # AntennaProcessor_v1 (concrete, ин…
- `strategies__ap_seq__seq_4_chunking_vram_002__v1` (seq_4_chunking_vram): ```  UserApp       AntennaProcessor_v1   Stream0   Stream1    Stream2   WelfordAccum     │                │               │          │          │            │     │ process(S,W)   │               │   …
- `strategies__ap_c2_container__s_5_plantuml_container_diagram_001__v1` (s_5_plantuml_container_diagram): ## 5. PlantUML: Container Diagram  ```plantuml @startuml AP_C2_Container !include <C4/C4_Container> LAYOUT_WITH_LEGEND() title AntennaProcessor — C2: Container Diagram  Person(user, "C++ Engineer / Py…

## Public-методы (3)

## Method 1: `process`

**Сигнатура** (`antenna_processor_v1.hpp:127`):
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

**Сигнатура** (`antenna_processor_v1.hpp:141`):
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

**Сигнатура** (`antenna_processor_v1.hpp:148`):
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

