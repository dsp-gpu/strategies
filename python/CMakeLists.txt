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

# ── Auto-deploy to DSP/Python/libs/ ─────────────────────────────────
option(DSP_DEPLOY_PYTHON_LIB "Auto-copy .so to DSP/Python/libs/" ON)
set(DSP_PYTHON_LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/../../DSP/Python/libs"
    CACHE PATH "Where to deploy compiled .so for tests")

if(DSP_DEPLOY_PYTHON_LIB)
  add_custom_command(TARGET dsp_strategies PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f
      "${DSP_PYTHON_LIB_DIR}/$<TARGET_FILE_NAME:dsp_strategies>"
    COMMENT "Remove stale dsp_strategies from DSP/Python/libs/")

  add_custom_command(TARGET dsp_strategies POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DSP_PYTHON_LIB_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy
      "$<TARGET_FILE:dsp_strategies>" "${DSP_PYTHON_LIB_DIR}/"
    COMMENT "Deploy dsp_strategies to DSP/Python/libs/")
endif()
