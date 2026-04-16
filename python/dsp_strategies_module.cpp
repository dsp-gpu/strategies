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

#if ENABLE_ROCM
#include "py_gpu_context.hpp"
#include "py_strategies_rocm.hpp"
#endif

PYBIND11_MODULE(dsp_strategies, m) {
    m.doc() = "dsp::strategies — full antenna processing pipeline (ROCm)\n\n"
              "Classes:\n"
              "  ROCmGPUContext        - GPU context (AMD ROCm)\n"
              "  AntennaProcessorTest - full antenna array pipeline (ROCm)\n"
              "  WeightGenerator      - adaptive weight generator (ROCm)\n";

#if ENABLE_ROCM
    py::class_<ROCmGPUContext>(m, "ROCmGPUContext",
        "ROCm GPU context (creates HIP backend for AMD GPU).")
        .def(py::init<int>(), py::arg("device_index") = 0)
        .def_property_readonly("device_name", &ROCmGPUContext::device_name)
        .def_property_readonly("device_index", &ROCmGPUContext::device_index);

    register_strategies_rocm(m);
#endif
}
