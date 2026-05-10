# Архитектурные паттерны репо `strategies`

> **Источник истины:** `strategies/.rag/_RAG.md` (теги `#pattern:Type:Class`, auto-inferred RAG_CLAUDE_C4 от 9.05).
> Brief'ы — из `key_classes:` того же манифеста (fallback из `rag_dsp.symbols`).
>
> Используется как источник для `dataset_v4` (collect_doc_deep подхватит Doc/Patterns.md).
> Alex: проверить + добавить руками то что не размечено в `_RAG.md tags:`.

## Pipeline

> Композиция операций в цепочку. Конфиг → Pipeline объект.


- **`strategies::Pipeline`** — `strategies/include/strategies/pipeline.hpp:36`
  - Owning-контейнер шагов + исполнитель. Хранит std::vector<std::unique_ptr<IPipelineStep>> all_steps_ (владение) и vector<Entry> entries_ (порядок исполнения). Каждый Entry — либо SEQUENTIAL (один шаг), либо PARALLEL (группа шагов, запускаемы
- **`strategies::PipelineContext`** — `strategies/include/strategies/i_pipeline_step.hpp:57`
  - Контекст исполнения pipeline'а: набор GPU-буферов (kBufX/kBufFftInput/kBufSpectrum/kBufMagnitudes/...), HIP stream, BackendType, AntennaProcessorConfig, hipEvent'ы для синхронизации между шагами. Передаётся в `IPipelineStep::Execute(PipelineContext&)`.

## Strategy

> Семейство взаимозаменяемых алгоритмов за общим интерфейсом (`IPipelineStep`).


- **`strategies::DebugStatsStep`** — `strategies/include/strategies/steps/debug_stats_step.hpp:30`
  - Параметризованный pipeline-шаг с тремя инстансами по точке наблюдения (DebugPoint::PRE_INPUT, POST_GEMM, POST_FFT). В каждой точке: ждёт нужное hipEvent (если есть), вызывает StatisticsProcessor::ComputeStatistics[Float] на соответствующем 
- **`strategies::MinMaxStep`** — `strategies/include/strategies/steps/minmax_step.hpp:26`
  - Pipeline-шаг сценария ALL_REQUIRED / GLOBAL_MINMAX. Запускает HIP- kernel global_minmax (1 block × n_ant blocks по второму измерению, 256 threads/block): один проход по kBufMagnitudes [n_ant × nFFT] даёт per-beam min, max, их позиции (bin) 
- **`strategies::OneMaxStep`** — `strategies/include/strategies/steps/one_max_step.hpp:26`
  - Pipeline-шаг сценария ALL_REQUIRED / ONE_MAX_PARABOLA. Запускает HIP-kernel one_max_no_phase (grid=(1, n_ant, 1), 256 threads/block): каждый y-блок ищет глобальный max в kBufMagnitudes по своему лучу, читает 3 точки kBufSpectrum вокруг пика
- **`strategies::AllMaximaStep`** — `strategies/include/strategies/steps/all_maxima_step.hpp:25`
  - Pipeline-шаг scenario_mode = ALL_REQUIRED / ALL_MAXIMA. Делегирует работу готовому AllMaximaPipelineROCm (модуль spectrum): читает kBufMagnitudes (|spectrum|) и kBufSpectrum (complex), вызывает Execute(...) с параметрами n_ant/nFFT/sample_r
- **`strategies::GemmStep`** — `strategies/include/strategies/steps/gemm_step.hpp:28`
  - Pipeline-шаг GEMM (complex single-precision): умножает входной сигнал d_S [n_samples × n_ant] на матрицу весов d_W [n_ant × n_ant], результат пишет в kBufX. Размерности M=n_samples, N=K=n_ant, alpha=(1,0), beta=(0,0). Stream берётся из hipb
- **`strategies::WindowFftStep`** — `strategies/include/strategies/steps/window_fft_step.hpp:31`
  - «Толстый» pipeline-шаг, объединяющий 4 операции: 1. hipMemsetAsync — занулить kBufFftInput полностью (для zero-padding); 2. fused-kernel hamming_pad_fused — наложить окно Hamming на kBufX и записать в первые n_samples элементов kBufFftInput
- **`strategies::IPipelineStep`** — `strategies/include/strategies/i_pipeline_step.hpp:70`
  - Pure-virtual интерфейс шага композиционного pipeline'а: расширяет `drv_gpu_lib::IGpuOperation` двумя методами — `Execute(PipelineContext&)` и `IsEnabled(AntennaProcessorConfig&)`. Конкретные шаги (GemmStep, WindowFftStep, OneMaxStep, AllMaximaStep, MinMaxStep, DebugStatsStep) реализуют этот интерфейс.
- **`strategies::PipelineStepBase`** — `strategies/include/strategies/i_pipeline_step.hpp:99`
  - Convenience-база для конкретных шагов: получает GpuContext из PipelineContext (а не напрямую). Уменьшает boilerplate, позволяет шагу не знать про GpuContext напрямую.

## Builder

> Поэтапное конструирование сложного объекта.


- **`strategies::PipelineBuilder`** — `strategies/include/strategies/pipeline_builder.hpp:28`
  - Промежуточный объект для декларативной сборки Pipeline. Имеет три метода-добавителя: add(step), add_if(cond, step), add_parallel(group, streams) — каждый возвращает *this для цепочки. После build() возвращает std::unique_ptr<Pipeline> и пер


## См. также

- `strategies/.rag/arch/C2_container.md`
- `strategies/.rag/arch/C3_component.md`
- `strategies/.rag/arch/C4_code.md`
- `MemoryBank/.architecture/DSP-GPU_Design_C4_Full.md`

---

*Сгенерировано из `_RAG.md` тегов. Alex редактирует руками + коммитит.*
