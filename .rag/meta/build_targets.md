<!-- type:meta_targets repo:strategies source:strategies/CMakeLists.txt -->

# Build Targets — strategies

## Targets

- **`DspStrategies`** (library)
  - PUBLIC: `DspCore::DspCore`, `DspSpectrum::DspSpectrum`, `DspStats::DspStats`, `DspSignalGenerators::DspSignalGenerators`, `DspHeterodyne::DspHeterodyne`, `DspLinalg::DspLinalg` (+1)

## BUILD-флаги (option)

- `DSP_STRATEGIES_BUILD_TESTS` (default `ON`) — Build tests
- `DSP_STRATEGIES_BUILD_PYTHON` (default `OFF`) — Build Python bindings

## Зависимости от DSP репо

- `core` — через `fetch_dsp_core()`
- `heterodyne` — через `fetch_dsp_heterodyne()`
- `linalg` — через `fetch_dsp_linalg()`
- `signal_generators` — через `fetch_dsp_signal_generators()`
- `spectrum` — через `fetch_dsp_spectrum()`
- `stats` — через `fetch_dsp_stats()`

## External find_package

- `hip` (required)
- `hipblas` (required)
