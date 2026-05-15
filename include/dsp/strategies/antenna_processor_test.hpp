#pragma once

// ============================================================================
// AntennaProcessorTest — step-by-step test-обёртка над AntennaProcessor_v1
//
// ЧТО:    Наследник AntennaProcessor_v1, открывающий каждую фазу pipeline'а
//         как отдельный публичный step_*-метод (step_0_prepare_input,
//         step_1_debug_input, step_2_gemm, step_3_debug_post_gemm,
//         step_4_window_fft, step_5_debug_post_fft, step_6_1/6_2/6_3 —
//         три post-FFT сценария). Каждый шаг исполняет ровно одну стадию,
//         синхронизирует stream и копирует результат на CPU как
//         std::vector<std::complex<float>> или AntennaResult — для
//         сравнения с NumPy/SciPy эталоном из Python-теста.
//
// ЗАЧЕМ:  В production AntennaProcessor_v1::process идёт сплошным потоком
//         (overlap по 7 streams, без точек копирования). Для отладки и
//         валидации каждой стадии нужно остановиться, забрать GPU-буфер
//         на CPU и сверить с эталоном — это невозможно через process().
//         Test-наследник даёт ровно эту возможность, не разрушая
//         production-путь и не дублируя логику фаз (do_*-методы — protected
//         в v1, доступны через наследование).
//
// ПОЧЕМУ: - Inheritance вместо отдельной копии: protected do_*-методы
//           v1 — точные блоки production-логики. Test переиспользует их,
//           гарантия что test и production исполняют один и тот же код.
//         - using AntennaProcessor_v1::AntennaProcessor_v1 — наследование
//           конструкторов, не нужно писать свой ctor.
//         - copy_buffer_to_cpu — приватный helper: hipMemcpy D2H
//           num_complex_elements × sizeof(complex<float>). Все step_-
//           методы используют для единообразного D2H.
//         - Каждый step_ синхронизирует stream/device — тесты ждут
//           завершения, тогда CPU читает корректные данные. В production
//           этих sync'ов нет (overlap), но в тесте — обязательны для
//           детерминированной сверки.
//         - step_6_X-методы временно подменяют scenario_mode через
//           const_cast (чтобы запустить ровно один сценарий) — допустимый
//           хак в тестовом контексте, восстанавливает значение после.
//         - process_full / process_full_managed_w — сценарии с разными
//           источниками весов (внешние upload через set_external_weights vs
//           caller-managed GPU pointer).
//         - step_0_signal_only — оптимизация повторного процессинга:
//             d_W уже на GPU (managed), не перезагружаем при каждом кадре.
//
// Использование:
//   AntennaProcessorTest test(backend, cfg);
//   test.set_external_weights(W);
//   test.step_0_prepare_input(d_S, test.get_managed_weights_ptr());
//   auto stats_in = test.step_1_debug_input();
//   auto X        = test.step_2_gemm();      // CPU-копия d_X для сверки с np.matmul
//   auto stats_X  = test.step_3_debug_post_gemm();
//   auto spec     = test.step_4_window_fft(); // CPU-копия для сверки с np.fft.fft
//   auto peaks    = test.step_6_1_one_max_parabola();
//
// История:
//   - Создан:  2026-03-07
//   - Изменён: 2026-05-01 (унификация формата шапки под dsp-asst RAG-индексер)
// ============================================================================

#include <dsp/strategies/antenna_processor_v1.hpp>
#include <dsp/strategies/weight_generator.hpp>

#include <complex>
#include <vector>

namespace dsp::strategies {

/**
 * @class AntennaProcessorTest
 * @brief Step-by-step расширение AntennaProcessor_v1 для отладки и Python-валидации.
 *
 * @note Наследник AntennaProcessor_v1 — переиспользует protected do_*-методы
 *       (исполняют ту же логику, что и production-process).
 * @note Каждый step_-метод синхронизирует stream и копирует результат на CPU.
 * @note Не для production (overhead D2H + sync на каждом шаге).
 * @see AntennaProcessor_v1 — production-родитель
 * @see WeightGenerator — генератор тестовых весовых матриц
 */
class AntennaProcessorTest : public AntennaProcessor_v1 {
public:
  using AntennaProcessor_v1::AntennaProcessor_v1;  // Inherit constructors

  // ========================================================================
  // Step-by-step API (Python-callable)
  // ========================================================================

  /**
   * @brief Step 0: Prepare input — store d_S, d_W pointers
   * @param d_S Входной сигнал [n_ant × n_samples] complex<float> на GPU.
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   * @param d_W Матрица весов [n_ant × n_ant] complex<float> на GPU.
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   */
  void step_0_prepare_input(const void* d_S, const void* d_W) {
    d_S_ = d_S;
    d_W_ = d_W;
  }

  /**
   * @brief Step 1: Debug point 2.1 — stats on d_S
   * @return Statistics on input signal
   *   @test_check result.pre_input_stats.size() == config().n_ant
   */
  AntennaResult step_1_debug_input() {
    AntennaResult result;
    do_debug_point_21(d_S_, result);
    hipStreamSynchronize(nullptr);  // Ensure stats complete
    return result;
  }

  /**
   * @brief Step 2: GEMM — X = W * S
   * @return d_X copied to CPU [n_ant x n_samples] complex<float>
   *   @test_check result.size() == config().n_ant * config().n_samples
   */
  std::vector<std::complex<float>> step_2_gemm() {
    do_gemm(d_S_, d_W_);
    hipDeviceSynchronize();
    return copy_buffer_to_cpu(get_d_X(),
        config().n_ant * config().n_samples);
  }

  /**
   * @brief Step 3: Debug point 2.2 — stats on d_X (после GEMM)
   * @return AntennaResult с post_gemm_stats — Welford по d_X.
   *   @test_check result.post_gemm_stats.size() == config().n_ant
   */
  AntennaResult step_3_debug_post_gemm() {
    AntennaResult result;
    do_debug_point_22(result);
    hipDeviceSynchronize();
    return result;
  }

  /**
   * @brief Step 4: Window + FFT
   * @return d_spectrum copied to CPU [n_ant x nFFT] complex<float>
   *   @test_check result.size() == config().n_ant * get_nFFT()
   */
  std::vector<std::complex<float>> step_4_window_fft() {
    do_window_fft();
    hipDeviceSynchronize();
    return copy_buffer_to_cpu(get_d_spectrum(),
        config().n_ant * get_nFFT());
  }

  /**
   * @brief Step 5: Debug point 2.3 — stats on |spectrum| (после FFT)
   * @return AntennaResult с post_fft_stats — Welford по магнитудам спектра.
   *   @test_check result.post_fft_stats.size() == config().n_ant
   */
  AntennaResult step_5_debug_post_fft() {
    AntennaResult result;
    do_debug_point_23(result);
    hipDeviceSynchronize();
    return result;
  }

  /**
   * @brief Step 6.1: OneMax + Parabola (no phase) — временно ставит scenario_mode = ONE_MAX_PARABOLA.
   * @return AntennaResult с one_max результатами per beam.
   *   @test_check result.one_max.size() == config().n_ant
   */
  AntennaResult step_6_1_one_max_parabola() {
    auto saved = config().scenario_mode;
    const_cast<AntennaProcessorConfig&>(config()).scenario_mode =
        PostFftScenarioMode::ONE_MAX_PARABOLA;
    AntennaResult result;
    do_run_post_fft_scenarios(result);
    const_cast<AntennaProcessorConfig&>(config()).scenario_mode = saved;
    return result;
  }

  /**
   * @brief Step 6.2: AllMaxima — временно ставит scenario_mode = ALL_MAXIMA.
   * @return AntennaResult с all_maxima.beams для всех антенн.
   *   @test_check result.all_maxima.beams.size() == config().n_ant
   */
  AntennaResult step_6_2_all_maxima() {
    auto saved = config().scenario_mode;
    const_cast<AntennaProcessorConfig&>(config()).scenario_mode =
        PostFftScenarioMode::ALL_MAXIMA;
    AntennaResult result;
    do_run_post_fft_scenarios(result);
    const_cast<AntennaProcessorConfig&>(config()).scenario_mode = saved;
    return result;
  }

  /**
   * @brief Step 6.3: GlobalMinMax — временно ставит scenario_mode = GLOBAL_MINMAX.
   * @return AntennaResult с min_max результатами per beam.
   *   @test_check result.min_max.size() == config().n_ant
   */
  AntennaResult step_6_3_global_minmax() {
    auto saved = config().scenario_mode;
    const_cast<AntennaProcessorConfig&>(config()).scenario_mode =
        PostFftScenarioMode::GLOBAL_MINMAX;
    AntennaResult result;
    do_run_post_fft_scenarios(result);
    const_cast<AntennaProcessorConfig&>(config()).scenario_mode = saved;
    return result;
  }

  /**
   * @brief Full pipeline (all steps + all scenarios) — делегирует в AntennaProcessor_v1::process(d_S_, d_W_).
   * @return Полный AntennaResult: статистики, пики, MinMax, метрики.
   *   @test_check result.success == true
   */
  AntennaResult process_full() {
    return process(d_S_, d_W_);
  }

  /**
   * @brief Full pipeline using external weights loaded via set_external_weights()
   *
   * Requires prior call to set_external_weights().
   * d_S must be set via step_0_prepare_input or step_0_signal_only.
   * @return Полный AntennaResult с использованием внутренней managed-копии весов.
   *   @test_check result.success == true
   */
  AntennaResult process_full_managed_w() {
    return process(d_S_, get_managed_weights_ptr());
  }

  /**
   * @brief Step 0 signal-only variant — uses pre-loaded managed weights
   *
   * Call after set_external_weights() to avoid re-uploading W on every frame.
   * Only updates d_S_; d_W_ is set to the internally managed GPU pointer.
   * @param d_S Входной сигнал [n_ant × n_samples] complex<float> на GPU (новый кадр).
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   */
  void step_0_signal_only(const void* d_S) {
    d_S_ = d_S;
    d_W_ = get_managed_weights_ptr();
  }

  // Getters for test access
  /**
   * @brief Возвращает текущий размер FFT (nextPow2 + zero-padding); для test-доступа.
   *
   * @return nFFT — степень двойки, рассчитанная в do_window_fft().
   *   @test_check result >= config().n_samples && (result & (result - 1)) == 0
   */
  uint32_t test_get_nFFT() const { return get_nFFT(); }

private:
  const void* d_S_ = nullptr;
  const void* d_W_ = nullptr;

  std::vector<std::complex<float>> copy_buffer_to_cpu(
      const void* d_buf, size_t num_complex_elements)
  {
    std::vector<std::complex<float>> host(num_complex_elements);
    hipMemcpy(host.data(), d_buf,
              num_complex_elements * sizeof(std::complex<float>),
              hipMemcpyDeviceToHost);
    return host;
  }
};

} // namespace dsp::strategies
