#pragma once

/**
 * @file result_types.hpp
 * @brief POD-результаты pipeline'а AntennaProcessor: OneMaxParabolaLite, MinMaxResult, PerfMetrics, AntennaResult.
 *
 * @note Тип B (technical header): чистые POD-struct'ы без логики, только
 *       defaults. Переиспользует MaxValue/AllMaximaBeamResult из spectrum
 *       и StatisticsResult/MedianResult из stats. Добавляет:
 *         - OneMaxParabolaLite — пик без фазы (Step2.1, lightweight);
 *         - MinMaxResult       — min+max за один проход (Step2.3, sizeof=36, выровнен);
 *         - PerfMetrics        — таймлайн всех шагов в мс;
 *         - AntennaResult      — агрегат всех debug-stats + scenario-результатов.
 *
 * История:
 *   - Создан:  2026-03-07
 *   - Изменён: 2026-05-01 (унификация формата шапки под dsp-asst RAG-индексер)
 */

#include <strategies/config/post_fft_scenario_mode.hpp>
#include <spectrum/types/spectrum_result_types.hpp>
#include <stats/statistics_types.hpp>

#include <cstdint>
#include <vector>

namespace strategies {

/**
 * @struct OneMaxParabolaLite
 * @brief Пик спектра без фазы (Step2.1) + 3-точечная параболическая интерполяция.
 * @note Отличие от OneMaxParabola (с фазой) — без поля phase, в ~2× меньше D2H-копирования.
 */
struct OneMaxParabolaLite {
  uint32_t beam_id          = 0;
  uint32_t bin_index        = 0;      ///< FFT bin of the peak
  float    magnitude        = 0.0f;   ///< |FFT[bin]|
  float    freq_offset      = 0.0f;   ///< Parabolic interpolation delta [-0.5, +0.5]
  float    refined_freq_hz  = 0.0f;   ///< (bin + delta) * fs / nFFT
};

/**
 * @struct MinMaxResult
 * @brief Глобальные MIN + MAX по лучу (Step2.3) + dynamic range в дБ.
 * @note sizeof = 36 байт (9 × uint32/float), pad для 32-байтового выравнивания.
 *       Device-side зеркало MinMaxResult_t в kernel global_minmax.
 */
struct MinMaxResult {
  uint32_t beam_id           = 0;
  // Min
  float    min_magnitude     = 0.0f;
  uint32_t min_bin           = 0;
  float    min_frequency_hz  = 0.0f;
  // Max
  float    max_magnitude     = 0.0f;
  uint32_t max_bin           = 0;
  float    max_frequency_hz  = 0.0f;
  // Derived
  float    dynamic_range_dB  = 0.0f;  ///< 20*log10(max/max(min, 1e-30))
  uint32_t pad               = 0;     ///< 32-byte alignment
};
// sizeof = 36 bytes (9 x uint32/float). GPU kernel uses MinMaxResult_t separately.

/**
 * @struct PerfMetrics
 * @brief Тайминги (в мс) каждого шага pipeline'а + total — для отчёта производительности.
 */
struct PerfMetrics {
  float debug_21_ms  = 0.0f;
  float gemm_ms      = 0.0f;
  float debug_22_ms  = 0.0f;
  float window_ms    = 0.0f;
  float fft_ms       = 0.0f;
  float debug_23_ms  = 0.0f;
  float step21_ms    = 0.0f;
  float step22_ms    = 0.0f;
  float step23_ms    = 0.0f;
  float total_ms     = 0.0f;
};

/**
 * @struct AntennaResult
 * @brief Агрегатный результат AntennaProcessor::process(): debug-stats + scenario-результаты + perf.
 * @note Заполняется шагами через ctx.result-> в Pipeline::Execute. Поля,
 *       соответствующие отключённым шагам, остаются пустыми (default-vector).
 */
struct AntennaResult {
  // Debug point statistics
  std::vector<statistics::StatisticsResult> pre_input_stats;  ///< 2.1: on d_S
  std::vector<statistics::StatisticsResult> post_gemm_stats;  ///< 2.2: on d_X
  std::vector<statistics::StatisticsResult> post_fft_stats;   ///< 2.3: on |spectrum|

  std::vector<statistics::MedianResult> pre_input_medians;
  std::vector<statistics::MedianResult> post_gemm_medians;
  std::vector<statistics::MedianResult> post_fft_medians;

  // Post-FFT scenario results
  std::vector<OneMaxParabolaLite>              one_max;      ///< Step2.1
  std::vector<antenna_fft::AllMaximaBeamResult> all_maxima;  ///< Step2.2
  std::vector<MinMaxResult>                    minmax;       ///< Step2.3

  PostFftScenarioMode scenario_mode = PostFftScenarioMode::ALL_REQUIRED;
  PerfMetrics perf;
};

}  // namespace strategies
