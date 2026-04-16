# ── DspStrategies Python bindings ─────────────────────────────────
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
execute_process(
  COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
  OUTPUT_VARIABLE pybind11_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(dsp_strategies dsp_strategies_module.cpp)
target_link_libraries(dsp_strategies PRIVATE DspStrategies::DspStrategies)
target_include_directories(dsp_strategies PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${FETCHCONTENT_SOURCE_DIR_DSPCORE}/python)
target_compile_definitions(dsp_strategies PRIVATE
  $<$<BOOL:${ENABLE_ROCM}>:ENABLE_ROCM=1>)
install(TARGETS dsp_strategies DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
