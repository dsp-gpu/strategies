/**
 * @file dsp_strategies_module.cpp
 * @brief pybind11 bindings for dsp::strategies
 *
 * Python API:
 *   import dsp_strategies
 *   proc = dsp_strategies.AntennaProcessorTest(ctx)
 *   result = proc.process(input_data)
 *
 * Экспортируемые классы:
 *   AntennaProcessorTest — antenna array pipeline (ROCm)
 *   WeightGenerator      — weight vector generator (ROCm)
 */

#include "py_helpers.hpp"

#include "py_gpu_context.hpp"
#include "py_strategies_rocm.hpp"

PYBIND11_MODULE(dsp_strategies, m) {
    m.doc() = "dsp::strategies — full antenna processing pipeline (ROCm)\n\n"
              "Classes:\n"
              "  ROCmGPUContext        - GPU context (AMD ROCm)\n"
              "  AntennaProcessorTest - full antenna array pipeline (ROCm)\n"
              "  WeightGenerator      - adaptive weight generator (ROCm)\n";

    // ROCmGPUContext зарегистрирован в dsp_core (один раз глобально).
    py::module_::import("dsp_core");

    register_strategies_rocm(m);
}
