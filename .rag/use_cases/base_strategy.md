---
schema_version: 1
kind: use_case
id: base_strategy
repo: strategies
title: "Base Strategy"
synonyms:
  ru:
    - []
  en:
    - []
primary_class: test_strategies::BaseStrategyTest
primary_method: GetName
related_classes:
  - strategies::statistics_processor
  - strategies::all_maxima_pipeline_rocm
  - strategies::antenna_processor
  - spectrum::fft_processor_rocm
  - strategies::antenna_processor_v1
related_use_cases:
  - spectrum__fir_basic__usecase__v1
  - radar__range_angle_basic__usecase__v1
  - radar__fm_basic__usecase__v1
maturity: stable
language: cpp
tags: []
ai_generated: false
human_verified: false
operator: alex
updated_at: 2026-05-13
---

# Use-case: Base Strategy

## Когда применять

_LLM-fallback: см. описание класса._

## Решение

Класс — `test_strategies::BaseStrategyTest`, метод `GetName`.

_Пример кода не найден в `tests/` или `examples/`._

## Граничные случаи

_Не определены (нет `@throws` в Doxygen primary_method)._

## Что делать дальше

- См. [spectrum__fir_basic__usecase__v1](./fir_basic.md)
- См. [radar__range_angle_basic__usecase__v1](./range_angle_basic.md)
- См. [radar__fm_basic__usecase__v1](./fm_basic.md)

## Ссылки

- Источник кода: `/home/alex/DSP-GPU/strategies/tests/test_base_strategy.hpp:1`
