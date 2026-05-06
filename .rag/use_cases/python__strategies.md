---
id: dsp__strategies_strategies__python_test_usecase__v1
type: python_test_usecase
source_path: strategies/python/t_strategies.py
primary_repo: strategies
module: strategies
uses_repos: ['strategies']
uses_external: ['numpy']
has_test_runner: false
is_opencl: false
line_count: 95
title: Тест биндингов strategies
tags: []
uses_pybind:
  - dsp_strategies.ROCmGPUContext
  - dsp_strategies.AntennaProcessorTest
top_functions:
  - check
synonyms_ru:
  - тест биндингов
  - тест strategies
  - тест rocm
  - тест python
  - тест gpu
inherits_block_id: strategies__rocm_gpu_context__class_overview__v1
block_refs:
  - strategies__rocm_gpu_context__class_overview__v1
ai_generated: false
human_verified: false
---

<!-- rag-block: id=dsp__strategies_strategies__python_test_usecase__v1 -->

# Python use-case: Тест биндингов strategies

## Цель

Проверка корректности Python-биндингов strategies для ROCm.

## Когда применять

Запускать после изменений в биндингах или ROCm-контексте.

## Используемые pybind-классы

| Класс / символ | Репо |
|---|---|
| `dsp_strategies.ROCmGPUContext` | strategies |
| `dsp_strategies.AntennaProcessorTest` | strategies |

## Внешние зависимости

numpy

## Solution (фрагмент кода)

```python
def check(name, condition, detail=""):
    global passed, failed
    if condition:
        passed += 1
        print(f"  [PASS] {name}" + (f"  ({detail})" if detail else ""))
    else:
        failed += 1
        print(f"  [FAIL] {name}" + (f"  ({detail})" if detail else ""))


print("=" * 60)
print("  dsp_strategies Python bindings test")
print("=" * 60)

# ── Test 1: Import ──────────────────────────────────────────────────
try:
    import dsp_strategies
    check("import dsp_strategies", True)
except ImportError as e:
    check("import dsp_strategies", False, str(e))
    sys.exit(1)

# ── Test 2: ROCmGPUContext ──────────────────────────────────────────
try:
    ctx = dsp_strategies.ROCmGPUContext(0)
```

## Connection (C++ ↔ Python)

- C++ class-card: `strategies__rocm_gpu_context__class_overview__v1`

## Метаданные

- **Source**: `strategies/python/t_strategies.py`
- **Строк кода**: 95
- **Top-функций**: 1
- **Test runner**: standalone (без runner)

<!-- /rag-block -->
