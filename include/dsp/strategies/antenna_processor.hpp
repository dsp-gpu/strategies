#pragma once

// ============================================================================
// AntennaProcessor — абстрактный фасад pipeline'а обработки антенной решётки
//
// ЧТО:    Pure-virtual базовый класс для модуля strategies. Один метод
//         process(d_S, d_W) запускает полный pipeline антенной обработки:
//         d_S (входной сигнал на GPU) → GEMM (X = W·S) → Window (Hamming) +
//         FFT (hipFFT batch) → post-FFT сценарии (OneMaxParabola /
//         AllMaxima / GlobalMinMax) → AntennaResult с агрегированными
//         статистиками, пиками и метриками. Дополнительно — runtime-
//         конфигурация (set_scenario_mode / set_*_stats / set_debug_mode).
//         Конкретная реализация — AntennaProcessor_v1 (ROCm).
//
// ЗАЧЕМ:  Это публичный API модуля strategies — Python-биндинги и тесты
//         работают через AntennaProcessor*, а не через v1-наследника.
//         Strategy-pattern даёт возможность подменять реализацию (v1 / v2 /
//         test) в run-time без изменения caller'а. Через runtime-сеттеры
//         (set_scenario_mode и др.) Python скрипты конфигурируют pipeline
//         без пересоздания процессора (важно для batch-обработки кадров).
//
// ПОЧЕМУ: - Layer 6 Ref03 (Facade + Strategy) — фасад потребителю,
//           Strategy для подмены v1/v2/Test. AntennaProcessorTest
//           наследуется от v1 и расширяет step-by-step API без правки
//           базы — OCP соблюдён.
//         - process(const void* d_S, const void* d_W) принимает GPU-
//           указатели типа void* — caller сам управляет H2D-загрузкой
//           (для overlap H2D ↔ compute в потоковом сценарии). Сигнатура
//           без std::vector — данные могут быть managed memory, mapped
//           OpenCL interop, hipMallocAsync — process не диктует backend.
//         - 5 setter'ов (scenario_mode, 3 × stats, debug_mode) — runtime
//           tuning без destroy/recreate. Конфиг изначально строится через
//           AntennaProcessorConfig в ctor конкретной реализации.
//         - config() возвращает const ref → caller читает текущий конфиг,
//           но не правит напрямую (инкапсуляция).
//         - gpu_id() — для multi-GPU сценариев (несколько AntennaProcessor
//           на разных GPU, отчёт ProfilingFacade per-GPU).
//
// Использование:
//   AntennaProcessorConfig cfg{ .n_ant=16, .n_samples=8000, .sample_rate=12e6f };
//   std::unique_ptr<AntennaProcessor> proc =
//       std::make_unique<AntennaProcessor_v1>(backend, cfg);
//   proc->set_scenario_mode(PostFftScenarioMode::ALL_REQUIRED);
//   AntennaResult result = proc->process(d_S, d_W);
//
// История:
//   - Создан:  2026-03-07
//   - Изменён: 2026-05-01 (унификация формата шапки под dsp-asst RAG-индексер)
// ============================================================================

#include <dsp/strategies/config/antenna_processor_config.hpp>
#include <dsp/strategies/result_types.hpp>

namespace dsp::strategies {

/**
 * @class AntennaProcessor
 * @brief Layer 6 Ref03 фасад: pure-virtual интерфейс pipeline'а антенной обработки.
 *
 * @note Pure interface — нельзя инстанцировать. Реализации: AntennaProcessor_v1, AntennaProcessorTest.
 * @note Не thread-safe. Один экземпляр = один владелец GPU-ресурсов pipeline'а.
 * @see AntennaProcessor_v1 — ROCm-реализация (concrete Strategy)
 * @see AntennaProcessorTest — step-by-step расширение для отладки и Python-валидации
 * @see AntennaProcessorConfig — POD-конфиг pipeline'а
 * @see PostFftScenarioMode — селектор post-FFT сценариев
 */
class AntennaProcessor {
public:
  virtual ~AntennaProcessor() = default;

  /**
   * @brief Запустить полный pipeline антенной обработки.
   * @param d_S Входной сигнал [n_ant × n_samples] complex<float> на GPU.
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   * @param d_W Матрица весов [n_ant × n_ant] complex<float> на GPU.
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   * @return Агрегированный результат: статистики, пики, MinMax, метрики производительности.
   *   @test_check result.success == true (для валидных d_S и d_W)
   */
  virtual AntennaResult process(const void* d_S, const void* d_W) = 0;

  /// Сменить активный post-FFT сценарий (ALL_REQUIRED / ONE_MAX_PARABOLA / ALL_MAXIMA / GLOBAL_MINMAX).
  virtual void set_scenario_mode(PostFftScenarioMode mode) = 0;
  /// Установить набор статистик для debug-точки 2.1 (pre-input, на d_S).
  virtual void set_pre_input_stats(StatisticsSet stats) = 0;
  /// Установить набор статистик для debug-точки 2.2 (post-GEMM, на d_X).
  virtual void set_post_gemm_stats(StatisticsSet stats) = 0;
  /// Установить набор статистик для debug-точки 2.3 (post-FFT, на |spectrum|).
  virtual void set_post_fft_stats(StatisticsSet stats) = 0;
  /// Включить debug-режим (D2H копии в debug-точках для CPU-валидации).
  virtual void set_debug_mode(bool enabled) = 0;

  /**
   * @brief Возвращает текущий конфиг pipeline'а (n_ant, n_samples, scenario_mode, ...).
   *
   * @return Const-ссылка на хранимый AntennaProcessorConfig.
   *   @test_check result.n_ant > 0 && result.n_samples > 0
   */
  virtual const AntennaProcessorConfig& config() const = 0;
  /**
   * @brief Возвращает идентификатор GPU, на котором работает процессор.
   *
   * @return GPU id (0..GetDeviceCount()-1).
   *   @test_check result >= 0
   */
  virtual int gpu_id() const = 0;
};

} // namespace dsp::strategies
