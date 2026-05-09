---
schema_version: 1
repo: strategies
arch_level: c4
tags:
  - "#level:c4"
  - "#repo:strategies"
  - "#layer:strategy"
  - "#pattern:Pipeline:StatisticsProcessor"
  - "#pattern:Pipeline:AntennaProcessorTest"
  - "#pattern:Pipeline:AllMaximaPipelineROCm"
  - "#pattern:Strategy:BaseStrategyTest"
  - "#pattern:Pipeline:AntennaProcessor_v1"
description: "C4 Code — реальные классы с паттернами GoF/SOLID для репо strategies."
---

# C4 Code — `strategies`

## Классы с паттернами проектирования

| Класс | Паттерн | Brief |
|-------|---------|-------|
| `StatisticsProcessor` | **Pipeline** |  |
| `AntennaProcessorTest` | **Pipeline** |  |
| `AllMaximaPipelineROCm` | **Pipeline** |  |
| `BaseStrategyTest` | **Strategy** | pipeline тест AntennaProcessor Генерирует сигнал, запускает полный process(), валидирует результаты. |
| `AntennaProcessor_v1` | **Pipeline** |  |
| `OneMaxStep` | **Strategy** |  |

## HIP-ядра (`kernels/rocm/`)

*kernels/rocm/ пуст или отсутствует.*

## Все key_classes (FQN список)

- `statistics::StatisticsProcessor` (32 методов)
- `strategies::StrategiesFloatApi` (8 методов)
- `strategies::NullCheckpointSave` (7 методов)
- `strategies::AntennaProcessorTest` (14 методов)
- `fft_processor::ComplexToMagPhaseROCm` (18 методов)
- `antenna_fft::AllMaximaPipelineROCm` (8 методов)
- `test_strategies::BaseStrategyTest` (3 методов)
- `drv_gpu_lib::GpuContext` (14 методов)
- `strategies::AntennaProcessor_v1` (38 методов)
- `strategies::OneMaxStep` (3 методов)
