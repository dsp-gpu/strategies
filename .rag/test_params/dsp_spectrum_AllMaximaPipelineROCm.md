---
schema_version: 1
repo: strategies
class_fqn: dsp::spectrum::AllMaximaPipelineROCm
file: E:/DSP-GPU/strategies/include/strategies/pipeline_context.hpp
line: 31
brief: "Обрабатывает массивы данных с использованием FFT на GPU ROCm для поиска максимумов в спектре."
methods_total: 0
methods_with_doxygen: 0
ai_generated: true
human_verified: false
parser_version: 2
synonyms_ru: ['PipelineROCm', 'FFTMaximaProcessor', 'AntennaSignalProcessor', 'GPUMaximaFinder']
synonyms_en: ['ROCmPipeline', 'FFTMaximaFinder', 'AntennaSignalProcessor', 'GPUMaximaFinder']
tags: ['GPU', 'ROCm', 'FFT', 'SignalProcessing', 'Radar', 'Pipeline']
---

# `dsp::spectrum::AllMaximaPipelineROCm` — карточка класса

> **Этот файл генерируется автоматически** командой `dsp-asst rag cards build --repo strategies --class AllMaximaPipelineROCm`.
> Не править руками — правки потеряются при следующем refresh.
> Источник правды — Doxygen-теги в `.hpp` + секции в `Doc/*.md`.

---

## Описание класса

<!-- rag-block: id=strategies__all_maxima_pipeline_rocm__class_overview__v1 -->

**ЧТО**: Обрабатывает массивы данных с использованием FFT на GPU ROCm для поиска максимумов в спектре.

**ЗАЧЕМ**: Оптимизирует вычисления поиска максимумов в спектре для радиолокационных задач, ускоряя обработку больших объемов данных.

**КАК**: Использует HIP для GPU-вычислений, оптимизирован для ROCm. Поддерживает батч-обработку и кэширование промежуточных данных.

**Пример**:
```cpp
#include "pipeline_context.hpp"
using namespace antenna_fft;
AllMaximaPipelineROCm pipeline;
pipeline.configure(1024, 2048);
std::vector<float> input(1024 * 2048);
// заполнение input
pipeline.process(input);
std::vector<int> maxima = pipeline.get_maxima();
```

<!-- /rag-block -->

## Public-методы (0)

_У класса AllMaximaPipelineROCm не найдено public-методов в header'е._

