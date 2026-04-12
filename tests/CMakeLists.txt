# ── DspStrategies tests ────────────────────────────────────────────
add_executable(test_strategies_main main.cpp)
target_link_libraries(test_strategies_main PRIVATE
  DspStrategies::DspStrategies DspCore::TestUtils)
target_include_directories(test_strategies_main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(test_strategies_main PRIVATE
  $<$<BOOL:${ENABLE_ROCM}>:ENABLE_ROCM=1>)
