# 🤖 CLAUDE — `strategies`

> Композиционные стратегии: `IPipelineStep` + `PipelineBuilder` + выбор реализаций.
> Зависит от: `core` + все рабочие модули. Глобальные правила → `../CLAUDE.md` + `.claude/rules/*.md`.

## 🎯 Что здесь

| Класс | Что делает |
|-------|-----------|
| `IPipelineStep` | Интерфейс шага пайплайна (Name, Process, Dependencies) |
| `PipelineBuilder` | Builder + DAG шагов → исполняемый Pipeline |
| `MedianStrategy` | Авто-выбор алгоритма median (histogram / radix sort) |
| `Pipeline` | Исполнитель: собирает DAG, запускает в правильном порядке |

## 📁 Структура

```
strategies/
├── include/dsp/strategies/
│   ├── i_pipeline_step.hpp
│   ├── pipeline_builder.hpp
│   ├── pipeline.hpp
│   ├── median_strategy.hpp
│   └── ...
├── src/
├── tests/                           # pipeline_v1, pipeline_v2 эталоны
└── python/dsp_strategies_module.cpp
```

## ⚠️ Специфика

- **DAG-based**: шаги объявляют зависимости по имени, builder строит DAG → топ-сорт → исполнение.
- **Immutable after Build()**: `PipelineBuilder.Build()` возвращает `Pipeline`, дальше не меняется.
- **Stream reuse**: один HIP stream на pipeline, шаги могут параллелиться через под-streams.
- **Profiling**: каждый шаг оборачивается в `ScopedProfileTimer` автоматически.

## 🚫 Запреты

- Не исполнять шаг напрямую — только через `Pipeline::Run()`.
- Не модифицировать `PipelineBuilder` после `Build()`.
- Не делать циклов в DAG — Builder должен проверять и бросать исключение.
- Не засовывать сюда конкретные ops (FFT, median) — только абстракции и orchestration.

## 🔗 Правила (path-scoped автоматически)

- `05-architecture-ref03.md` — Strategy + Builder
- `14-cpp-style.md` + `15-cpp-testing.md`
- `11-python-bindings.md`
