---
schema_version: 1
repo: strategies
version: 0.1.0
layer: compute
maturity: alpha
purpose: "TODO: AI-fill — назначение репо strategies"

modules:
  public:                               # auto: include/<repo>/*
    - checkpoint
    - config
    - interfaces
    - kernels
    - steps
  internal:                             # auto: src/* кроме include
    - strategies

key_classes:                            # auto: top по test_params
  - fqn: strategies::StrategiesFloatApi
    brief: "Standalone post-FFT computations from CPU float magnitudes"
    maturity: alpha
    methods: 8
    test_params_rows: 16
    test_params: test_params/strategies_StrategiesFloatApi.md
  - fqn: antenna_fft::AllMaximaPipelineROCm
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 8
    test_params_rows: 9
    test_params: test_params/antenna_fft_AllMaximaPipelineROCm.md
  - fqn: fft_processor::ComplexToMagPhaseROCm
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 18
    test_params_rows: 6
    test_params: test_params/fft_processor_ComplexToMagPhaseROCm.md
  - fqn: drv_gpu_lib::GpuContext
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 14
    test_params_rows: 4
    test_params: test_params/drv_gpu_lib_GpuContext.md
  - fqn: strategies::AntennaProcessorTest
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 14
    test_params_rows: 3
    test_params: test_params/strategies_AntennaProcessorTest.md
  - fqn: strategies::DebugStatsStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 3
    test_params_rows: 2
    test_params: test_params/strategies_DebugStatsStep.md
  - fqn: strategies::MinMaxStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 3
    test_params_rows: 2
    test_params: test_params/strategies_MinMaxStep.md
  - fqn: strategies::OneMaxStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 3
    test_params_rows: 2
    test_params: test_params/strategies_OneMaxStep.md
  - fqn: strategies::AllMaximaStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 2
    test_params_rows: 2
    test_params: test_params/strategies_AllMaximaStep.md
  - fqn: strategies::GemmStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 2
    test_params_rows: 1
    test_params: test_params/strategies_GemmStep.md
  - fqn: strategies::WindowFftStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 2
    test_params_rows: 1
    test_params: test_params/strategies_WindowFftStep.md
  - fqn: strategies::Pipeline
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 1
    test_params_rows: 1
    test_params: test_params/strategies_Pipeline.md
  - fqn: strategies::WeightGenerator
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 1
    test_params_rows: 1
    test_params: test_params/strategies_WeightGenerator.md
  - fqn: strategies::AntennaProcessor_v1
    brief: "@ingroup grp_strategies"
    maturity: alpha
    methods: 38
    test_params_rows: 0
    test_params: test_params/strategies_AntennaProcessor_v1.md
  - fqn: statistics::StatisticsProcessor
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 32
    test_params_rows: 0
    test_params: test_params/statistics_StatisticsProcessor.md
  - fqn: PyAntennaProcessorTest
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 21
    test_params_rows: 0
    test_params: test_params/PyAntennaProcessorTest.md
  - fqn: test_strategies::StrategyTestBase
    brief: "Базовый класс всех тестов антенной стратегии"
    maturity: alpha
    methods: 10
    test_params_rows: 0
    test_params: test_params/test_strategies_StrategyTestBase.md
  - fqn: strategies::NullCheckpointSave
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 7
    test_params_rows: 0
    test_params: test_params/strategies_NullCheckpointSave.md
  - fqn: test_strategies::TimingPerStepTest
    brief: "Быстрый замер времени каждого шага AntennaProcessor"
    maturity: alpha
    methods: 6
    test_params_rows: 0
    test_params: test_params/test_strategies_TimingPerStepTest.md
  - fqn: test_strategies::DebugStepTest
    brief: "Пошаговый тест AntennaProcessor (debug режим)"
    maturity: alpha
    methods: 4
    test_params_rows: 0
    test_params: test_params/test_strategies_DebugStepTest.md

test_params_summary:
  classes_with_params: 10
  methods_with_params: 17
  ready_for_autotest:  0
  partial_coverage:    31
  no_status:           0
  total_rows:          31

repo_stats:
  total_symbols: 389
  public_classes: 59
  total_files: 75

depends_on:                              # TODO: ручная разметка после deps таблицы
  internal: []
  external: []

used_by: []                              # TODO: AI-fill из других _RAG.md

python_modules:                          # TODO: auto from pybind_bindings
  - TODO

tags: []                                 # TODO: AI-fill

notes: []                                # TODO: AI-fill из ai_summary

ai_generated_at: 2026-05-09T05:27:59Z
ai_model: TODO (auto-fields only, AI-brief pending)
ai_sections: []
parser_version: 1
---

# strategies

## Назначение
*(TODO: AI-fill через ollama qwen3:8b)*

## Ключевые классы
*(автогенерируется из YAML key_classes выше)*

## Дополнительная документация
- [../Doc/](../Doc/)

<!-- ⚙️ Auto-generated by generate_rag_manifest.py — отредактируй и закоммить. -->
