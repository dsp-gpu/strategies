<!-- type:meta_cmake_specific repo:strategies inherits:dsp_gpu__root__meta_cmake_common__v1 -->

# CMake Specific — strategies

```yaml
inherits: dsp_gpu__root__meta_cmake_common__v1
specific_only: true
target: DspStrategies
description: "Antenna array processing pipelines"
adds_find_package: [hip, hipblas]
adds_links: [DspCore::DspCore, DspHeterodyne::DspHeterodyne, DspLinalg::DspLinalg, DspSignalGenerators::DspSignalGenerators, DspSpectrum::DspSpectrum, DspStats::DspStats, roc::hipblas]
```

## Project

- **Target**: `DspStrategies`
- **Описание**: Antenna array processing pipelines

## Уникальные find_package

```cmake
find_package(hip REQUIRED)
find_package(hipblas REQUIRED)
```

## Линкуемые библиотеки

```cmake
target_link_libraries(DspStrategies PUBLIC
  DspCore::DspCore
  DspHeterodyne::DspHeterodyne
  DspLinalg::DspLinalg
  DspSignalGenerators::DspSignalGenerators
  DspSpectrum::DspSpectrum
  DspStats::DspStats
  roc::hipblas
)
```

## Исходники (3 файлов)

```cmake
target_sources(DspStrategies PRIVATE
  src/strategies/src/antenna_processor_v1.cpp
  src/strategies/src/weight_generator.cpp
  src/strategies/src/strategies_float_api.cpp
)
```

## Прочие специфичные строки (15)

```cmake
<TARGET>::<TARGET>
DESCRIPTION "Antenna array processing pipelines"
fetch_dsp_heterodyne()
fetch_dsp_linalg()
fetch_dsp_signal_generators()
fetch_dsp_spectrum()
fetch_dsp_stats()
find_package(hip    REQUIRED)
find_package(hipblas REQUIRED)
roc::hipblas
src/strategies/src/antenna_processor_v1.cpp
src/strategies/src/strategies_float_api.cpp
src/strategies/src/weight_generator.cpp
target_compile_definitions(<TARGET> PUBLIC ENABLE_ROCBLAS=1)
target_link_libraries(<TARGET>
```

