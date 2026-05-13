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
  - fqn: dsp::strategies::StrategiesFloatApi
    brief: "Standalone post-FFT computations from CPU float magnitudes"
    maturity: alpha
    methods: 8
    test_params_rows: 16
    test_params: test_params/dsp_strategies_StrategiesFloatApi.md
  - fqn: dsp::spectrum::AllMaximaPipelineROCm
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 8
    test_params_rows: 9
    test_params: test_params/dsp_spectrum_AllMaximaPipelineROCm.md
  - fqn: dsp::spectrum::ComplexToMagPhaseROCm
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 18
    test_params_rows: 6
    test_params: test_params/dsp_spectrum_ComplexToMagPhaseROCm.md
  - fqn: drv_gpu_lib::GpuContext
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 14
    test_params_rows: 4
    test_params: test_params/drv_gpu_lib_GpuContext.md
  - fqn: dsp::strategies::AntennaProcessorTest
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 14
    test_params_rows: 3
    test_params: test_params/dsp_strategies_AntennaProcessorTest.md
  - fqn: dsp::strategies::DebugStatsStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 3
    test_params_rows: 2
    test_params: test_params/dsp_strategies_DebugStatsStep.md
  - fqn: dsp::strategies::MinMaxStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 3
    test_params_rows: 2
    test_params: test_params/dsp_strategies_MinMaxStep.md
  - fqn: dsp::strategies::OneMaxStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 3
    test_params_rows: 2
    test_params: test_params/dsp_strategies_OneMaxStep.md
  - fqn: dsp::strategies::AllMaximaStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 2
    test_params_rows: 2
    test_params: test_params/dsp_strategies_AllMaximaStep.md
  - fqn: dsp::strategies::GemmStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 2
    test_params_rows: 1
    test_params: test_params/dsp_strategies_GemmStep.md
  - fqn: dsp::strategies::WindowFftStep
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 2
    test_params_rows: 1
    test_params: test_params/dsp_strategies_WindowFftStep.md
  - fqn: dsp::strategies::Pipeline
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 1
    test_params_rows: 1
    test_params: test_params/dsp_strategies_Pipeline.md
  - fqn: dsp::strategies::WeightGenerator
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 1
    test_params_rows: 1
    test_params: test_params/dsp_strategies_WeightGenerator.md
  - fqn: dsp::strategies::AntennaProcessor_v1
    brief: "@ingroup grp_strategies"
    maturity: alpha
    methods: 38
    test_params_rows: 0
    test_params: test_params/dsp_strategies_AntennaProcessor_v1.md
  - fqn: dsp::stats::StatisticsProcessor
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 32
    test_params_rows: 0
    test_params: test_params/dsp_stats_StatisticsProcessor.md
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
  - fqn: dsp::strategies::NullCheckpointSave
    brief: "TODO: AI-fill"
    maturity: alpha
    methods: 7
    test_params_rows: 0
    test_params: test_params/dsp_strategies_NullCheckpointSave.md
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

architecture_files:                       # auto: arch_files generator
  - .rag/arch/C2_container.md
  - .rag/arch/C3_component.md
  - .rag/arch/C4_code.md
tags:                                    # auto-inferred (RAG_CLAUDE_C4)
  - "#layer:strategy"
  - "#repo:strategies"
  - "#namespace:dsp_strategies"
  - "#namespace:antenna_fft"
  - "#namespace:fft_processor"
  - "#pattern:Strategy:DebugStatsStep"
  - "#pattern:Strategy:MinMaxStep"
  - "#pattern:Strategy:OneMaxStep"
  - "#pattern:Strategy:AllMaximaStep"
  - "#pattern:Strategy:GemmStep"
  - "#pattern:Strategy:WindowFftStep"
  - "#pattern:Pipeline:Pipeline"
  - "#pattern:Pipeline:PipelineContext"
  - "#pattern:Strategy:IPipelineStep"
  - "#pattern:Strategy:PipelineStepBase"
  - "#pattern:Builder:PipelineBuilder"

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
