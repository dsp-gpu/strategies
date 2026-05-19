#!/usr/bin/env python3
"""
Standalone test for dsp_strategies Python bindings (ROCm).
Запуск: python test_strategies.py

Tests:
  1. Import + ROCmGPUContext
  2. AntennaProcessorTest — create
  3. AntennaProcessorTest — step_0_prepare_input (smoke)
  4. WeightGenerator — generate_uniform
"""

import sys
import os
import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
build_python = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                             '..', 'build', 'python')
if os.path.isdir(build_python):
    sys.path.insert(0, build_python)

# Fallback: DSP/Python/libs/ (центральная папка со всеми .so)
_DSP_PY = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                         "..", "..", "DSP", "Python"))
if _DSP_PY not in sys.path:
    sys.path.insert(0, _DSP_PY)
try:
    from common.gpu_loader import GPULoader
    GPULoader.setup_path()
except ImportError:
    pass

passed = 0
failed = 0

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
    import dsp_core
    ctx = dsp_core.ROCmGPUContext(0)
    check("ROCmGPUContext(0)", True, f"device={ctx.device_name}")
except Exception as e:
    check("ROCmGPUContext(0)", False, str(e))
    sys.exit(1)

# ── Test 3: AntennaProcessorTest — create ───────────────────────────
try:
    proc = dsp_strategies.AntennaProcessorTest(
        ctx, n_ant=5, n_samples=4000,
        sample_rate=12e6, signal_frequency_hz=2e6)
    check("AntennaProcessorTest create", True)
except Exception as e:
    check("AntennaProcessorTest create", False, str(e))

# ── Test 4: step_0_prepare_input (smoke) ────────────────────────────
try:
    n_ant = 5
    n_samples = 4000
    # d_S: n_ant * n_samples complex64 (signal array)
    d_S = (np.random.randn(n_ant * n_samples) +
           1j * np.random.randn(n_ant * n_samples)).astype(np.complex64)
    # W: n_ant complex64 (weight vector)
    W = np.ones(n_ant, dtype=np.complex64) / n_ant
    proc.step_0_prepare_input(d_S, W)
    check("step_0_prepare_input", True)
except Exception as e:
    check("step_0_prepare_input", False, str(e))

# ── Test 5: step_2_gemm ────────────────────────────────────────────
try:
    gemm_result = proc.step_2_gemm()
    check("step_2_gemm", gemm_result is not None and len(gemm_result) > 0,
          f"len={len(gemm_result)}, dtype={gemm_result.dtype}")
except Exception as e:
    check("step_2_gemm", False, str(e))

# ── Summary ─────────────────────────────────────────────────────────
print("=" * 60)
total = passed + failed
print(f"  Results: {passed}/{total} passed" +
      (f", {failed} FAILED" if failed else " -- ALL PASSED"))
print("=" * 60)

sys.exit(1 if failed else 0)
