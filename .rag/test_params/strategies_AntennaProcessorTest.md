---
schema_version: 1
repo: strategies
class_fqn: strategies::AntennaProcessorTest
file: E:/DSP-GPU/strategies/include/strategies/antenna_processor_test.hpp
line: 24
brief: "Предоставляет пошаговый интерфейс для тестирования обработки сигналов на GPU с синхронизацией ROCm."
methods_total: 13
methods_with_doxygen: 13
ai_generated: true
human_verified: false
parser_version: 2
synonyms_ru: ['тестовый процессор', 'пошаговая обработка', 'GPU-тестирование', 'ROCm-тестирование']
synonyms_en: ['test processor', 'step-by-step processing', 'GPU testing', 'ROCm testing']
tags: ['GPU', 'тестирование', 'обработка сигналов', 'ROCm', 'пошаговый API']
---

# `strategies::AntennaProcessorTest` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class AntennaProcessorTest`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__antenna_processor_test__class_overview__v1 -->

**ЧТО**: Предоставляет пошаговый интерфейс для тестирования обработки сигналов на GPU с синхронизацией ROCm.

**ЗАЧЕМ**: Для детальной валидации этапов обработки сигналов (GEMM, FFT, пост-FFT сценарии) с отладочными выводами.

**КАК**: Использует HIP для GPU-синхронизации, разделяет логику на этапы с явной порядковой зависимостью. Кэширует результаты этапов для повторного использования.

**Пример**:
```cpp
#include "strategies/antenna_processor_test.hpp"
using namespace strategies;

int main() {
  auto backend = ...;
  AntennaProcessorTest test(backend, config);
  test.step_0_prepare_input(d_S, d_W);
  auto debug = test.step_1_debug_input();
  auto gemm = test.step_2_gemm();
  auto fft = test.step_4_window_fft();
  auto result = test.process_full();
  return 0;
}
```

<!-- /rag-block -->

## Связанные секции из Doc/

- `strategies__gpu__s_8_001__v1` (s_8): ## 8. Файловая структура  ``` strategies/ ├── CMakeLists.txt ├── include/ │   ├── antenna_processor.hpp          # AntennaProcessor (abstract) │   ├── antenna_processor_v1.hpp       # AntennaProcessor…
- `strategies__farrow_pipeline__pipeline_data_flow_012__v1` (pipeline_data_flow):     std::vector<float> get_magnitudes();      // [n_beams * nFFT]     std::vector<float> get_peaks_freq_hz();   // [n_top] частоты пиков     std::vector<float> get_peaks_mag();       // [n_top] магнит…
- `strategies__api__s_2_antennaprocessor_v1_001__v1` (s_2_antennaprocessor_v1): ## 2. AntennaProcessor_v1  **Файл**: `strategies/include/antenna_processor_v1.hpp` **Реализация**: `strategies/src/antenna_processor_v1.cpp`  Конкретная ROCm-реализация. 4 HIP-потока, hipBLAS, hipFFT,…
- `strategies__api__s_3_antennaprocessortest__v1` (s_3_antennaprocessortest): ## 3. AntennaProcessorTest  **Файл**: `strategies/include/antenna_processor_test.hpp`  > ⚠️ Только для тестов! В production не использовать.  Наследник `AntennaProcessor_v1`, открывает step-by-step AP…
- `strategies__gpu__api_002__v1` (api): ### 7.1 C++ API  #### AntennaProcessor (abstract)  ```cpp namespace strategies {  class AntennaProcessor { public:   virtual ~AntennaProcessor() = default;    // Основной вызов — d_S и d_W уже на GPU …

## Public-методы (13)

## Method 1: `step_0_prepare_input`

**Сигнатура** (`antenna_processor_test.hpp:93`):
```cpp
void step_0_prepare_input(const void* d_S, const void* d_W) { d_S_ = d_S; d_W_ = d_W;
```

**Параметры**:
- `d_S` — `const void*` *(pointer)* *(void\*)*
- `d_W` — `const void*` *(pointer)* *(void\*)*

**Doxygen-источник**:
```cpp
/**

   * @brief Step 0: Prepare input — store d_S, d_W pointers

   * @param d_S Входной сигнал [n_ant × n_samples] complex<float> на GPU.

   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }

   * @param d_W Матрица весов [n_ant × n_ant] complex<float> на GPU.

   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }

   */
```

## Method 2: `step_1_debug_input`

**Сигнатура** (`antenna_processor_test.hpp:103`):
```cpp
AntennaResult step_1_debug_input() { AntennaResult result; do_debug_point_21(d_S_, result); #if ENABLE_ROCM hipStreamSynchronize(nullptr); // Ensure stats complete #endif return result;
```

**Возвращает**: `AntennaResult`

**Doxygen-источник**:
```cpp
/**

   * @brief Step 1: Debug point 2.1 — stats on d_S

   * @return Statistics on input signal

   *   @test_check result.pre_input_stats.size() == config().n_ant

   */
```

## Method 3: `step_2_gemm`

**Сигнатура** (`antenna_processor_test.hpp:117`):
```cpp
std::vector<std::complex<float>> step_2_gemm() { do_gemm(d_S_, d_W_); #if ENABLE_ROCM hipDeviceSynchronize(); #endif return copy_buffer_to_cpu(get_d_X(), config().n_ant * config().n_samples);
```

**Возвращает**: `std::vector<std::complex<float>>`

**Doxygen-источник**:
```cpp
/**

   * @brief Step 2: GEMM — X = W * S

   * @return d_X copied to CPU [n_ant x n_samples] complex<float>

   *   @test_check result.size() == config().n_ant * config().n_samples

   */
```

## Method 4: `step_3_debug_post_gemm`

**Сигнатура** (`antenna_processor_test.hpp:131`):
```cpp
AntennaResult step_3_debug_post_gemm() { AntennaResult result; do_debug_point_22(result); #if ENABLE_ROCM hipDeviceSynchronize(); #endif return result;
```

**Возвращает**: `AntennaResult`

**Doxygen-источник**:
```cpp
/**

   * @brief Step 3: Debug point 2.2 — stats on d_X (после GEMM)

   * @return AntennaResult с post_gemm_stats — Welford по d_X.

   *   @test_check result.post_gemm_stats.size() == config().n_ant

   */
```

## Method 5: `step_4_window_fft`

**Сигнатура** (`antenna_processor_test.hpp:145`):
```cpp
std::vector<std::complex<float>> step_4_window_fft() { do_window_fft(); #if ENABLE_ROCM hipDeviceSynchronize(); #endif return copy_buffer_to_cpu(get_d_spectrum(), config().n_ant * get_nFFT());
```

**Возвращает**: `std::vector<std::complex<float>>`

**Doxygen-источник**:
```cpp
/**

   * @brief Step 4: Window + FFT

   * @return d_spectrum copied to CPU [n_ant x nFFT] complex<float>

   *   @test_check result.size() == config().n_ant * get_nFFT()

   */
```

## Method 6: `step_5_debug_post_fft`

**Сигнатура** (`antenna_processor_test.hpp:159`):
```cpp
AntennaResult step_5_debug_post_fft() { AntennaResult result; do_debug_point_23(result); #if ENABLE_ROCM hipDeviceSynchronize(); #endif return result;
```

**Возвращает**: `AntennaResult`

**Doxygen-источник**:
```cpp
/**

   * @brief Step 5: Debug point 2.3 — stats on |spectrum| (после FFT)

   * @return AntennaResult с post_fft_stats — Welford по магнитудам спектра.

   *   @test_check result.post_fft_stats.size() == config().n_ant

   */
```

## Method 7: `step_6_1_one_max_parabola`

**Сигнатура** (`antenna_processor_test.hpp:173`):
```cpp
AntennaResult step_6_1_one_max_parabola() { auto saved = config().scenario_mode; const_cast<AntennaProcessorConfig&>(config()).scenario_mode = PostFftScenarioMode::ONE_MAX_PARABOLA; AntennaResult result; do_run_post_fft_scenarios(result); const_cast<AntennaProcessorConfig&>(config()).scenario_mode = saved; return result;
```

**Возвращает**: `AntennaResult`

**Doxygen-источник**:
```cpp
/**

   * @brief Step 6.1: OneMax + Parabola (no phase) — временно ставит scenario_mode = ONE_MAX_PARABOLA.

   * @return AntennaResult с one_max результатами per beam.

   *   @test_check result.one_max.size() == config().n_ant

   */
```

## Method 8: `step_6_2_all_maxima`

**Сигнатура** (`antenna_processor_test.hpp:188`):
```cpp
AntennaResult step_6_2_all_maxima() { auto saved = config().scenario_mode; const_cast<AntennaProcessorConfig&>(config()).scenario_mode = PostFftScenarioMode::ALL_MAXIMA; AntennaResult result; do_run_post_fft_scenarios(result); const_cast<AntennaProcessorConfig&>(config()).scenario_mode = saved; return result;
```

**Возвращает**: `AntennaResult`

**Doxygen-источник**:
```cpp
/**

   * @brief Step 6.2: AllMaxima — временно ставит scenario_mode = ALL_MAXIMA.

   * @return AntennaResult с all_maxima.beams для всех антенн.

   *   @test_check result.all_maxima.beams.size() == config().n_ant

   */
```

## Method 9: `step_6_3_global_minmax`

**Сигнатура** (`antenna_processor_test.hpp:203`):
```cpp
AntennaResult step_6_3_global_minmax() { auto saved = config().scenario_mode; const_cast<AntennaProcessorConfig&>(config()).scenario_mode = PostFftScenarioMode::GLOBAL_MINMAX; AntennaResult result; do_run_post_fft_scenarios(result); const_cast<AntennaProcessorConfig&>(config()).scenario_mode = saved; return result;
```

**Возвращает**: `AntennaResult`

**Doxygen-источник**:
```cpp
/**

   * @brief Step 6.3: GlobalMinMax — временно ставит scenario_mode = GLOBAL_MINMAX.

   * @return AntennaResult с min_max результатами per beam.

   *   @test_check result.min_max.size() == config().n_ant

   */
```

## Method 10: `process_full`

**Сигнатура** (`antenna_processor_test.hpp:218`):
```cpp
AntennaResult process_full() { return process(d_S_, d_W_);
```

**Возвращает**: `AntennaResult`

**Doxygen-источник**:
```cpp
/**

   * @brief Full pipeline (all steps + all scenarios) — делегирует в AntennaProcessor_v1::process(d_S_, d_W_).

   * @return Полный AntennaResult: статистики, пики, MinMax, метрики.

   *   @test_check result.success == true

   */
```

## Method 11: `process_full_managed_w`

**Сигнатура** (`antenna_processor_test.hpp:230`):
```cpp
AntennaResult process_full_managed_w() { return process(d_S_, get_managed_weights_ptr());
```

**Возвращает**: `AntennaResult`

**Doxygen-источник**:
```cpp
/**

   * @brief Full pipeline using external weights loaded via set_external_weights()

   *

   * Requires prior call to set_external_weights().

   * d_S must be set via step_0_prepare_input or step_0_signal_only.

   * @return Полный AntennaResult с использованием внутренней managed-копии весов.

   *   @test_check result.success == true

   */
```

## Method 12: `step_0_signal_only`

**Сигнатура** (`antenna_processor_test.hpp:242`):
```cpp
void step_0_signal_only(const void* d_S) { d_S_ = d_S; d_W_ = get_managed_weights_ptr();
```

**Параметры**:
- `d_S` — `const void*` *(pointer)* *(void\*)*

**Doxygen-источник**:
```cpp
/**

   * @brief Step 0 signal-only variant — uses pre-loaded managed weights

   *

   * Call after set_external_weights() to avoid re-uploading W on every frame.

   * Only updates d_S_; d_W_ is set to the internally managed GPU pointer.

   * @param d_S Входной сигнал [n_ant × n_samples] complex<float> на GPU (новый кадр).

   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }

   */
```

## Method 13: `test_get_nFFT`

**Сигнатура** (`antenna_processor_test.hpp:254`):
```cpp
uint32_t test_get_nFFT() const { return get_nFFT();
```

**Возвращает**: `uint32_t`

**Doxygen-источник**:
```cpp
/**

   * @brief Возвращает текущий размер FFT (nextPow2 + zero-padding); для test-доступа.

   *

   * @return nFFT — степень двойки, рассчитанная в do_window_fft().

   *   @test_check result >= config().n_samples && (result & (result - 1)) == 0

   */
```


## Python API

**Pybind модуль**: `dsp_strategies` · **Класс Python**: `AntennaProcessorTest` · **Wrapper C++**: `PyAntennaProcessorTest`

_Источник биндинга_: `strategies/python/py_strategies_rocm.hpp`

**Конструктор**: `py::init<ROCmGPUContext&, uint32_t, uint32_t, float, float, bool>()`

| Python | C++ | Overload |
|---|---|---|
| `step_0_prepare_input` | `PyAntennaProcessorTest::step_0_prepare_input` | — |
| `step_1_debug_input` | `PyAntennaProcessorTest::step_1_debug_input` | — |
| `step_2_gemm` | `PyAntennaProcessorTest::step_2_gemm` | — |
| `step_3_debug_post_gemm` | `PyAntennaProcessorTest::step_3_debug_post_gemm` | — |
| `step_4_window_fft` | `PyAntennaProcessorTest::step_4_window_fft` | — |
| `step_5_debug_post_fft` | `PyAntennaProcessorTest::step_5_debug_post_fft` | — |
| `step_6_1_one_max_parabola` | `PyAntennaProcessorTest::step_6_1_one_max_parabola` | — |
| `step_6_2_all_maxima` | `PyAntennaProcessorTest::step_6_2_all_maxima` | — |
| `step_6_3_global_minmax` | `PyAntennaProcessorTest::step_6_3_global_minmax` | — |
| `process_full` | `PyAntennaProcessorTest::process_full` | — |
| `set_external_weights` | `PyAntennaProcessorTest::set_external_weights` | — |
| `step_0_signal_only` | `PyAntennaProcessorTest::step_0_signal_only` | — |
| `process_full_managed_w` | `PyAntennaProcessorTest::process_full_managed_w` | — |
