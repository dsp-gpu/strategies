#pragma once

// ============================================================================
// all_test.hpp — точка входа в тесты модуля strategies
//
// ЧТО:    Агрегирует все тесты strategies: pipeline, step-profiling,
//         benchmark-streams, base-strategy (OOP/SOLID), debug-steps.
// ЗАЧЕМ:  Единственный файл, который нужно подключить в src/main.cpp для
//         запуска всех тестов модуля. Выключение теста = закомментировать
//         одну строку include.
// ПОЧЕМУ: Следует правилу 15: main.cpp не вызывает тесты напрямую, только
//         через all_test.hpp каждого модуля.
//
// История: Создан: 2026-03-07
// ============================================================================

/**
 * @file all_test.hpp
 * @brief Entry point for strategies module tests.
 * @note Не публичный API. Подключается из src/main.cpp.
 */

#include "test_strategies_pipeline.hpp"
#include "test_strategies_step_profiling.hpp"
#include "test_strategies_benchmark_streams.hpp"

// ── New test infrastructure (OOP/SOLID/GRASP/GoF) ───────────────────────────
#include "test_base_strategy.hpp"
#include "test_debug_steps.hpp"
// #include "strategies_profiling_benchmark.hpp"  // включить отдельно если нужно
#include "timing_per_step_test.hpp"  // T4 — генерит timing_*.json в Results/strategies/

#include <core/backends/rocm/rocm_backend.hpp>
#include <core/services/console_output.hpp>

namespace strategies_all_test {


inline drv_gpu_lib::IBackend* GetTestBackend() {
  static drv_gpu_lib::ROCmBackend backend;
  if (!backend.IsInitialized()) {
    backend.Initialize(0);
  }
  return &backend;
}


inline void run() {
  auto& con = drv_gpu_lib::ConsoleOutput::GetInstance();
  if (!con.IsRunning()) con.Start();
  int gpu_id = 0;

  con.Print(gpu_id, "Strategies", "");
  con.Print(gpu_id, "Strategies", "════════════════════════════════════════════════════════════");
  con.Print(gpu_id, "Strategies", " Strategies Tests (ROCm)");
  con.Print(gpu_id, "Strategies", "════════════════════════════════════════════════════════════");

  auto* backend = GetTestBackend();
  test_strategies::test_full_pipeline(backend);
  test_strategies::test_external_weights(backend);
  // test_strategies_profiling::run_step_profiling(backend);
  test_strategies_benchmark_streams::run_benchmark_streams(backend);

  // ── New tests: OOP/SOLID/GRASP/GoF framework ────────────────────────────
  test_base_strategy::run_sin_only(backend);          // T1: быстрый smoke-тест
  // test_base_strategy::run_all_variants(backend);   // T1: все 4 сигнала
  // test_debug_steps::run_all(backend);              // T2: step-by-step debug

  // ── T4: TimingPerStepTest → JSON в DSP/Results/strategies/ ─────────────
  {
    auto params = test_strategies::AntennaTestParams::Small();
    params.output_dir = "../DSP/Results/strategies/";
    auto sig = test_strategies::SignalStrategyFactory::Create(
                  test_strategies::SignalVariant::SIN);
    test_strategies::TimingPerStepTest t4(backend, std::move(sig), params);
    t4.Run();
  }

  con.Print(gpu_id, "Strategies", "════════════════════════════════════════════════════════════");
  con.Print(gpu_id, "Strategies", " All Strategies tests PASSED ✅");
  con.Print(gpu_id, "Strategies", "════════════════════════════════════════════════════════════");
}

}  // namespace strategies_all_test
