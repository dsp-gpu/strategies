---
schema_version: 1
kind: use_case
id: base_strategy
repo: strategies
title: "Как протестировать базовую стратегию обработки сигналов на GPU"
synonyms:
  ru:
    - "тестирование базовой стратегии"
    - "проверка обработки сигналов"
    - "тестирование стратегии обработки"
    - "тестирование pipeline"
    - "тестирование AntennaProcessor"
    - "проверка стратегии на GPU"
    - "тестирование шаблона обработки"
    - "тестирование стратегии сигналов"
  en:
    - "test base strategy"
    - "signal processing strategy test"
    - "antenna processor test"
    - "strategy pipeline testing"
    - "gpu strategy testing"
    - "base strategy validation"
    - "signal processing validation"
    - "strategy template testing"
primary_class: test_strategies::BaseStrategyTest
primary_method: GetName
related_classes:
related_use_cases:
  - core__zero_copy__usecase__v1
  - heterodyne__heterodyne_basic__usecase__v1
  - linalg__capon_rocm__usecase__v1
maturity: stable
language: cpp
tags: [strategies, testing, strategy, signal_processing, gpu, pipeline, antenna, validation, base, template]
ai_generated: true
human_verified: false
operator: ai
updated_at: 2026-05-06
---

# Use-case: Как протестировать базовую стратегию обработки сигналов на GPU

## Когда применять

Когда нужно создать тест для пользовательской стратегии обработки сигналов, используя готовый шаблон полного pipeline

## Решение

Класс — `test_strategies::BaseStrategyTest`, метод `GetName`.

_Пример кода не найден в `tests/` или `examples/`._

## Граничные случаи

_Не определены (нет `@throws` в Doxygen primary_method)._

## Что делать дальше

- См. [core__zero_copy__usecase__v1](./zero_copy.md)
- См. [heterodyne__heterodyne_basic__usecase__v1](./heterodyne_basic.md)
- См. [linalg__capon_rocm__usecase__v1](./capon_rocm.md)

## Ссылки

- Источник кода: `E:/DSP-GPU/strategies/tests/test_base_strategy.hpp:1`
