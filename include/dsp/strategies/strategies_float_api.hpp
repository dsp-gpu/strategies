#pragma once

// ============================================================================
// StrategiesFloatApi — публичный CPU-vector API post-FFT вычислений модуля strategies
//
// ЧТО:    Тонкий C++-фасад поверх strategies kernel'ов для caller'ов, у
//         которых данные magnitudes уже на CPU как std::vector<float>.
//         Три метода: OneMaxParabolaFromFloat (один пик + парабола на луч),
//         GlobalMinMaxFromFloat (min+max+dynamic_range на луч),
//         AllMaximaFromMagnitudes (все локальные максимумы на луч через
//         dsp::spectrum::AllMaximaPipelineROCm). Каждый метод сам выделяет
//         GPU-буфер, делает H2D, запускает kernel, делает D2H, освобождает.
//
// ЗАЧЕМ:  Часть caller'ов (Python биндинги, unit-тесты, external tools) уже
//         имеют magnitudes на CPU — им не нужно держать persistent
//         AntennaProcessor с его 7 streams и буферами. Этот класс даёт
//         lightweight «standalone» доступ к тем же strategies-kernel'ам,
//         без overhead'а полного pipeline'а. Симметричен CPU wrapper'ам
//         StatisticsProcessor (одинаковый паттерн hipMalloc→H2D→kernel→
//         D2H→hipFree).
//
// ПОЧЕМУ: - GpuContext (Ref03 Layer 1) — компиляция strategies-ядер ровно
//           один раз в ctor (disk-cached HSACO по CompileKey). При
//           повторном создании на том же GPU/архитектуре kernel'ы берутся
//           с диска без recompile (Phase C4 kernel_cache_v2).
//         - Move/copy запрещены — owns GpuContext + AllMaximaPipelineROCm +
//           hipFunction_t handles + hipStream_t. Копирование = chaos с
//           lifetime kernel module'ов.
//         - Forward declaration drv_gpu_lib::GpuContext — strategies_float_api
//           используется во многих местах (Python binding, тесты), не
//           тащим тяжёлый <core/interface/gpu_context.hpp> в header.
//         - persistent stream_ + persistent kernel handles → между
//           вызовами Free's и Compile's нет, только H2D/D2H буфера.
//         - #if ENABLE_ROCM на весь header — на non-ROCm сборках класс
//           недоступен (нет stub'а). Caller'ы строят с тем же гардом.
//         - Phase C4 рефакторинг (2026-04-22): был 337-строчный header-inline,
//           разбили на .hpp/.cpp + delegated kernel-compile в GpuContext.
//
// Использование:
//   dsp::strategies::StrategiesFloatApi api(backend);
//   std::vector<float> mags(n_ant * nFFT);  // готовые модули спектра
//   auto peaks = api.OneMaxParabolaFromFloat(mags, n_ant, nFFT, sample_rate);
//   auto mm    = api.GlobalMinMaxFromFloat(mags, n_ant, nFFT, sample_rate);
//   auto all   = api.AllMaximaFromMagnitudes(mags, n_ant, nFFT, sample_rate);
//
// История:
//   - Создан:  2026-03-12
//   - Изменён: 2026-05-01 (унификация формата шапки под dsp-asst RAG-индексер;
//                          ранее: 2026-04-22 — split на .hpp/.cpp, GpuContext, Phase C4)
// ============================================================================

#if ENABLE_ROCM

#include <dsp/strategies/result_types.hpp>
#include <core/interface/i_backend.hpp>
#include <dsp/spectrum/pipelines/all_maxima_pipeline_rocm.hpp>

#include <hip/hip_runtime.h>

#include <vector>
#include <memory>
#include <cstdint>

// Forward declaration — keeps <core/interface/gpu_context.hpp> out of
// downstream includes (strategies_float_api is used by several callers).
namespace drv_gpu_lib { class GpuContext; }

namespace dsp::strategies {

/**
 * @class StrategiesFloatApi
 * @brief Standalone post-FFT вычисления из CPU std::vector<float> магнитуд.
 *
 * @note Move/copy запрещены — owns GpuContext + kernel handles + AllMaximaPipelineROCm.
 * @note Только ROCm (#if ENABLE_ROCM). На non-ROCm сборках класс недоступен.
 * @note Каждый метод: hipMalloc → H2D → kernel → D2H → hipFree (нет persistent GPU state).
 * @note Kernel'ы компилируются один раз в ctor через GpuContext (disk-cached HSACO).
 * @see GpuContext — Ref03 Layer 1, kernel compile/cache
 * @see AllMaximaPipelineROCm — переиспользуется для AllMaximaFromMagnitudes
 * @see AntennaProcessor_v1 — полный pipeline для GPU-resident данных
 */
class StrategiesFloatApi {
public:
  explicit StrategiesFloatApi(drv_gpu_lib::IBackend* backend);
  ~StrategiesFloatApi();

  StrategiesFloatApi(const StrategiesFloatApi&)            = delete;
  StrategiesFloatApi& operator=(const StrategiesFloatApi&) = delete;

  /**
   * @brief Find one peak + parabolic interpolation per beam (CPU float input)
   *
   * @param mags        Flat float magnitudes [n_ant * nFFT]
   * @param n_ant       Number of beams / antennas
   * @param nFFT        FFT size per beam
   *   @test { range=[8..4194304], value=1024, pattern=power_of_2, error_values=[-1, 9000000, 3.14] }
   * @param sample_rate Sampling frequency [Hz]
   *   @test { range=[1.0..1e9], value=10e6, unit="Гц", error_values=[0.0, 2e9, null] }
   * @return Vector of OneMaxParabolaLite (one per beam)
   *   @test_check result.size() == n_ant
   */
  std::vector<OneMaxParabolaLite> OneMaxParabolaFromFloat(
      const std::vector<float>& mags,
      uint32_t n_ant, uint32_t nFFT, float sample_rate);

  /**
   * @brief Compute global MIN + MAX + dynamic_range per beam (CPU float input)
   *
   * @param mags        Flat float magnitudes [n_ant * nFFT]
   * @param n_ant       Number of beams / antennas
   * @param nFFT        FFT size per beam
   *   @test { range=[8..4194304], value=1024, pattern=power_of_2, error_values=[-1, 9000000, 3.14] }
   * @param sample_rate Sampling frequency [Hz]
   *   @test { range=[1.0..1e9], value=10e6, unit="Гц", error_values=[0.0, 2e9, null] }
   * @return Vector of MinMaxResult (one per beam)
   *   @test_check result.size() == n_ant
   */
  std::vector<MinMaxResult> GlobalMinMaxFromFloat(
      const std::vector<float>& mags,
      uint32_t n_ant, uint32_t nFFT, float sample_rate);

  /**
   * @brief Find all local maxima per beam (CPU float input)
   *
   * @param mags                Flat float magnitudes [beam_count * nFFT]
   * @param beam_count          Number of beams
   *   @test { range=[1..50000], value=128, unit="лучей/каналов", error_values=[-1, 100000, 3.14] }
   * @param nFFT                FFT size per beam
   *   @test { range=[8..4194304], value=1024, pattern=power_of_2, error_values=[-1, 9000000, 3.14] }
   * @param sample_rate         Sampling frequency [Hz]
   *   @test { range=[1.0..1e9], value=10e6, unit="Гц", error_values=[0.0, 2e9, null] }
   * @param dest                Output destination (CPU or GPU)
   *   @test { size=[100..1300000], value=6000, unit="elements", error_values=[-1, 3000000, 3.14] }
   * @param search_start        First bin to search (default 1 to skip DC)
   *   @test { range=[0..1000000], value=0, error_values=[-1, 2000000, 3.14] }
   * @param search_end          Last bin (0 = nFFT/2)
   *   @test { range=[0..1300000], value=8192, error_values=[-1, 3000000, 3.14] }
   * @param max_maxima_per_beam Max number of maxima per beam
   *   @test { range=[1..50000], value=128, error_values=[-1, 100000, 3.14] }
   * @return AllMaximaResult with beams vector
   *   @test_check result.beams.size() == beam_count
   */
  dsp::spectrum::AllMaximaResult AllMaximaFromMagnitudes(
      const std::vector<float>& mags,
      uint32_t beam_count, uint32_t nFFT, float sample_rate,
      dsp::spectrum::OutputDestination dest = dsp::spectrum::OutputDestination::CPU,
      uint32_t search_start = 1,
      uint32_t search_end = 0,
      uint32_t max_maxima_per_beam = 1000);

private:
  static constexpr uint32_t kBlockSize = 256;

  drv_gpu_lib::IBackend* backend_ = nullptr;
  hipStream_t            stream_  = nullptr;

  std::unique_ptr<drv_gpu_lib::GpuContext>            ctx_;
  std::unique_ptr<dsp::spectrum::AllMaximaPipelineROCm> all_maxima_;

  hipFunction_t minmax_kernel_  = nullptr;
  hipFunction_t one_max_kernel_ = nullptr;
};

} // namespace dsp::strategies

#endif  // ENABLE_ROCM
