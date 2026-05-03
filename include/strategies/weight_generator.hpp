#pragma once

// ============================================================================
// WeightGenerator — генератор Delay-and-Sum матрицы весов для beamforming
//
// ЧТO:    Утилитный класс с двумя static-методами:
//         generate_delay_and_sum(params) — синтез [n_ant × n_ant] комплексной
//         матрицы весов W по формуле W[beam][ant] = exp(-j·2π·f0·τ[ant])/√N,
//         где τ[ant] = tau_base + ant·tau_step (равномерная линейка задержек);
//         upload_to_gpu(backend, W) — H2D-перенос матрицы на GPU через
//         IBackend, возвращает device-pointer (caller освобождает сам).
//         Параметры — POD-struct WeightParams (n_ant, f0, tau_base, tau_step).
//
// ЗАЧЕМ:  В pipeline AntennaProcessor нужна матрица весов W для GEMM
//         X = W·S. В production W обычно загружается извне (из калибровки),
//         но для тестов / синтетики / Quick-Look нужен быстрый способ
//         получить аналитическую Delay-and-Sum формулу. Один статический
//         метод вместо разрозненных скриптов на Python — единый источник
//         истины для эталонной формулы (Doc/Modules/strategies/Full.md).
//
// ПОЧЕМУ: - Static методы — у класса нет состояния, instance не нужен.
//           Вынесено в класс (а не свободные функции) для группировки в
//           одной namespace и для возможного будущего расширения
//           (HammingWeights / ChebyshevWeights — те же сигнатуры).
//         - Возвращается std::vector<std::complex<float>> row-major flat —
//           совместимо с раскладкой hipBLAS (column-major через transpose-
//           флаги, либо row-major напрямую). Один и тот же layout, что
//           ожидает AntennaProcessor_v1::set_external_weights.
//         - upload_to_gpu принимает void* backend (а не drv_gpu_lib::IBackend*)
//           — намеренно: weight_generator.hpp не должен тащить
//           <core/interface/i_backend.hpp> в публичный header. Caster
//           в .cpp — разовый, риск минимален.
//         - 1/√N нормировка внутри generate_delay_and_sum — гарантирует
//           ‖W‖_F-инвариант, чтобы post-GEMM магнитуды совпадали с CPU
//           эталоном NumPy без дополнительных scale-факторов.
//         - WeightParams.tau_step default = 100e-6s (микросекундный шаг) —
//           выбран как типичный PRI для радарных сценариев DSP-GPU.
//
// Использование:
//   strategies::WeightParams p;
//   p.n_ant = 16; p.f0 = 2.0e6; p.tau_step = 50e-6;
//   auto W = strategies::WeightGenerator::generate_delay_and_sum(p);
//   void* d_W = strategies::WeightGenerator::upload_to_gpu(backend, W);
//   processor.process(d_S, d_W);
//   backend->Free(d_W);
//
// История:
//   - Создан:  2026-03-07
//   - Изменён: 2026-05-01 (унификация формата шапки под dsp-asst RAG-индексер)
// ============================================================================

#include <complex>
#include <vector>
#include <cstdint>

namespace strategies {

/**
 * @struct WeightParams
 * @brief Параметры генерации Delay-and-Sum матрицы весов.
 *
 * @note POD-struct. tau[ant] = tau_base + ant · tau_step.
 *       f0 — несущая частота сигнала, для exp(-j·2π·f0·τ).
 */
struct WeightParams {
  uint32_t n_ant    = 5;
  double   f0       = 2.0e6;      ///< Несущая частота сигнала, Гц
  double   tau_base = 0.0;        ///< Базовая задержка, с
  double   tau_step = 100e-6;     ///< Шаг задержки между антеннами, с
};

/**
 * @class WeightGenerator
 * @brief Генератор Delay-and-Sum матриц весов и H2D-загрузчик на GPU.
 *
 * @note Stateless: только static-методы, instance не нужен.
 * @note Формула: W[beam][ant] = exp(-j·2π·f0·τ[ant]) / √N_ant.
 * @see WeightParams — входные параметры
 * @see AntennaProcessor_v1::set_external_weights — потребитель матрицы
 */
class WeightGenerator {
public:
  /**
   * @brief Сгенерировать матрицу весов Delay-and-Sum.
   * @param params Количество антенн, частота, базовая задержка и шаг.
   *   @test_ref WeightParams
   * @return Flat row-major матрица [n_ant × n_ant] complex<float>.
   *
   * W[beam][ant] = exp(-j·2π·f0·τ[ant]) / √N_ant, τ[ant] = tau_base + ant·tau_step.
   *   @test_check result.size() == params.n_ant * params.n_ant
   */
  static std::vector<std::complex<float>> generate_delay_and_sum(
      const WeightParams& params);

  /**
   * @brief Залить матрицу весов на GPU через IBackend.
   * @param backend Указатель на drv_gpu_lib::IBackend (передаётся как void*).
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr] }
   * @param weights Flat [n_ant × n_ant] complex<float> матрица.
   * @return Device-pointer (caller обязан освободить через backend->Free()).
   */
  static void* upload_to_gpu(
      void* backend,  // drv_gpu_lib::IBackend*
      const std::vector<std::complex<float>>& weights);
};

}  // namespace strategies
