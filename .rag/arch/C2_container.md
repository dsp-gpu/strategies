---
schema_version: 1
repo: strategies
arch_level: c2
tags:
  - "#level:c2"
  - "#repo:strategies"
  - "#layer:strategy"
  - "#namespace:strategies"
  - "#namespace:test_strategies"
  - "#namespace:drv_gpu_lib"
description: "C2 Container — namespace tree и зависимости репо strategies."
---

# C2 Container — `strategies` (layer=strategy)

## Namespaces (top по числу классов)

- `strategies`
- `test_strategies`
- `drv_gpu_lib`
- `fft_processor`
- `antenna_fft`
- `statistics`
- `test_strategies_benchmark_streams`

## Public modules (`include/strategies/`)

- `checkpoint/`
- `config/`
- `interfaces/`
- `kernels/`
- `steps/`

## Зависимости (depends_on)

`core` → `spectrum` → `stats`

## Используется (used_by)

`radar`, `DSP`

## Top key_classes

| Class | Namespace | Methods | TestParams |
|-------|-----------|--------:|-----------:|
| `StatisticsProcessor` | `statistics` | 32 | 36 |
| `StrategiesFloatApi` | `strategies` | 8 | 19 |
| `NullCheckpointSave` | `strategies` | 7 | 16 |
| `AntennaProcessorTest` | `strategies` | 14 | 14 |
| `ComplexToMagPhaseROCm` | `fft_processor` | 18 | 12 |
