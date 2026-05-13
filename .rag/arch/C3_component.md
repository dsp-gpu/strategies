---
schema_version: 1
repo: strategies
arch_level: c3
tags:
  - "#level:c3"
  - "#repo:strategies"
  - "#layer:strategy"
  - "#namespace:statistics"
  - "#namespace:dsp_strategies"
  - "#namespace:fft_processor"
description: "C3 Component — key classes и интерфейсы репо strategies."
---

# C3 Component — `strategies`

## Key classes (top-10 по test_params)

### `dsp::stats::StatisticsProcessor`

- **Namespace:** `statistics`
- **Методы:** 32, **test_params rows:** 36
- **Brief:** *(описание не задано)*

### `dsp::strategies::StrategiesFloatApi`

- **Namespace:** `strategies`
- **Методы:** 8, **test_params rows:** 19
- **Brief:** post-FFT computations from CPU float magnitudes Compiles strategies kernels once in the constructor via GpuContext (disk-cached HSAC

### `dsp::strategies::NullCheckpointSave`

- **Namespace:** `strategies`
- **Методы:** 7, **test_params rows:** 16
- **Brief:** *(описание не задано)*

### `dsp::strategies::AntennaProcessorTest`

- **Namespace:** `strategies`
- **Методы:** 14, **test_params rows:** 14
- **Brief:** *(описание не задано)*

### `dsp::spectrum::ComplexToMagPhaseROCm`

- **Namespace:** `fft_processor`
- **Методы:** 18, **test_params rows:** 12
- **Brief:** *(описание не задано)*

### `dsp::spectrum::AllMaximaPipelineROCm`

- **Namespace:** `antenna_fft`
- **Методы:** 8, **test_params rows:** 10
- **Brief:** *(описание не задано)*

### `test_strategies::BaseStrategyTest`

- **Namespace:** `test_strategies`
- **Методы:** 3, **test_params rows:** 10
- **Brief:** pipeline тест AntennaProcessor Генерирует сигнал, запускает полный process(), валидирует результаты. Является шаблоном для тестирования любой стратегии обработки.

### `drv_gpu_lib::GpuContext`

- **Namespace:** `drv_gpu_lib`
- **Методы:** 14, **test_params rows:** 7
- **Brief:** *(описание не задано)*

### `dsp::strategies::AntennaProcessor_v1`

- **Namespace:** `strategies`
- **Методы:** 38, **test_params rows:** 4
- **Brief:** *(описание не задано)*

### `dsp::strategies::OneMaxStep`

- **Namespace:** `strategies`
- **Методы:** 3, **test_params rows:** 4
- **Brief:** *(описание не задано)*

## Интерфейсы (наследуемые)

- `drv_gpu_lib::IBackend` (потенциальных реализаций: 6)
- `dsp::strategies::ICheckpointSave` (потенциальных реализаций: 2)
- `dsp::strategies::IPipelineStep` (потенциальных реализаций: 2)
- `dsp::strategies::IPostFftScenario` (потенциальных реализаций: 0)
- `test_strategies::ISignalStrategy` (потенциальных реализаций: 5)

