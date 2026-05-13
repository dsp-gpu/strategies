cmake_minimum_required(VERSION 3.25)
project(DspStrategies VERSION 0.1.0
  DESCRIPTION "Antenna array processing pipelines"
  LANGUAGES CXX HIP)

# ── Опции ──────────────────────────────────────────────────────────
option(DSP_STRATEGIES_BUILD_TESTS  "Build tests"            ON)
option(DSP_STRATEGIES_BUILD_PYTHON "Build Python bindings"  OFF)

# ── Зависимости ────────────────────────────────────────────────────
find_package(hip    REQUIRED)   # lowercase — Linux case-sensitive!
find_package(hipblas REQUIRED)  # для GEMM в GemmStep/AntennaProcessor

include(cmake/fetch_deps.cmake)
fetch_dsp_core()
fetch_dsp_spectrum()
fetch_dsp_stats()
fetch_dsp_signal_generators()
fetch_dsp_heterodyne()
fetch_dsp_linalg()

# ── Библиотека ─────────────────────────────────────────────────────
add_library(DspStrategies STATIC)
add_library(DspStrategies::DspStrategies ALIAS DspStrategies)

# PIC нужен для Python bindings (.so линкует static lib)
set_target_properties(DspStrategies PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_sources(DspStrategies PRIVATE
  src/strategies/antenna_processor_v1.cpp
  src/strategies/weight_generator.cpp
  src/strategies/strategies_float_api.cpp
)

target_include_directories(DspStrategies
  PUBLIC
    # Стандарт DSP-GPU: #include <dsp/strategies/steps/...>, <dsp/strategies/config/...>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    kernels/
    src
)
target_link_libraries(DspStrategies
  PUBLIC
    DspCore::DspCore
    DspSpectrum::DspSpectrum
    DspStats::DspStats
    DspSignalGenerators::DspSignalGenerators
    DspHeterodyne::DspHeterodyne
    DspLinalg::DspLinalg
    roc::hipblas
)
target_compile_features(DspStrategies PUBLIC cxx_std_17)

if(ENABLE_ROCM)
  target_compile_definitions(DspStrategies PUBLIC ENABLE_ROCM=1)
  # ENABLE_ROCBLAS: linalg→core GpuContext::GetRocblasHandleRaw()
  target_compile_definitions(DspCore PUBLIC ENABLE_ROCBLAS=1)
endif()

# ── Git Version ────────────────────────────────────────────────────
find_package(Git QUIET)
if(NOT GIT_EXECUTABLE)
    set(GIT_EXECUTABLE git)
endif()

add_custom_target(git_version_DspStrategies ALL
    COMMAND ${CMAKE_COMMAND}
        -D SRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -D BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
        -D MODULE_PREFIX=DSPSTRATEGIES
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cmake
    COMMENT "[DspStrategies] Checking git version..."
    VERBATIM
)
add_dependencies(DspStrategies git_version_DspStrategies)

# ── Install + Export ────────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS DspStrategies EXPORT DspStrategiesTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/version.h"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/DspStrategies
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/version.json"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/DspStrategies
)

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/DspStrategiesConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/DspStrategiesConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DspStrategies
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/DspStrategiesConfigVersion.cmake"
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion
)
install(EXPORT DspStrategiesTargets NAMESPACE DspStrategies::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DspStrategies
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/DspStrategiesConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/DspStrategiesConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DspStrategies
)

if(DSP_STRATEGIES_BUILD_TESTS)
  add_subdirectory(tests)
endif()
if(DSP_STRATEGIES_BUILD_PYTHON)
  add_subdirectory(python)
endif()
