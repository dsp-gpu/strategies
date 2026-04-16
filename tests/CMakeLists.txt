# ── DspStrategies tests ────────────────────────────────────────────

# HSA runtime: нужен для ZeroCopy (hsa_interop.cpp через DspCore)
find_library(HSA_RUNTIME_LIB_STRAT
  NAMES hsa-runtime64
  PATH_SUFFIXES lib lib64
  REQUIRED
)

add_executable(test_strategies_main main.cpp)
target_link_libraries(test_strategies_main PRIVATE
  DspStrategies::DspStrategies
  DspCore::TestUtils
)
target_link_options(test_strategies_main PRIVATE "LINKER:--no-as-needed")
target_link_libraries(test_strategies_main PRIVATE ${HSA_RUNTIME_LIB_STRAT})
target_include_directories(test_strategies_main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(test_strategies_main PRIVATE
  $<$<BOOL:${ENABLE_ROCM}>:ENABLE_ROCM=1>)
