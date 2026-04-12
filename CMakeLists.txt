cmake_minimum_required(VERSION 3.25)
project(DspStrategies VERSION 0.1.0
  DESCRIPTION "Antenna array processing pipelines"
  LANGUAGES CXX HIP)

# ── Опции ──────────────────────────────────────────────────────────
option(DSP_STRATEGIES_BUILD_TESTS  "Build tests"            ON)
option(DSP_STRATEGIES_BUILD_PYTHON "Build Python bindings"  OFF)

# ── Зависимости ────────────────────────────────────────────────────
find_package(hip REQUIRED)   # lowercase — Linux case-sensitive!

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

target_sources(DspStrategies PRIVATE
  src/strategies/src/antenna_processor_v1.cpp
  src/strategies/src/weight_generator.cpp
)

target_include_directories(DspStrategies
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    src/strategies/include
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
)
target_compile_features(DspStrategies PUBLIC cxx_std_17)

# ── Install + Export ────────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS DspStrategies EXPORT DspStrategiesTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

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
