#pragma once

/**
 * @file strategies_float_api.hpp
 * @brief CPU-vector API for post-FFT strategies computations (declarations only)
 *
 * Provides convenience wrappers accepting std::vector<float> magnitudes:
 *   - OneMaxParabolaFromFloat  — one peak + parabolic interpolation per beam
 *   - GlobalMinMaxFromFloat    — global MIN + MAX + dynamic range per beam
 *   - AllMaximaFromMagnitudes  — all local maxima per beam via AllMaximaPipeline
 *
 * Pattern (same as StatisticsProcessor CPU wrappers):
 *   hipMalloc → H2D → GPU kernel → D2H → hipFree
 *
 * Phase C4 of kernel_cache_v2: split from 337-line header-inline to .hpp/.cpp.
 * Kernel compilation delegated to GpuContext (clean-slate v2 cache).
 *
 * IMPORTANT: ROCm-only. All code under #if ENABLE_ROCM.
 *
 * @date 2026-03-12  (split 2026-04-22)
 */

#if ENABLE_ROCM

#include <strategies/result_types.hpp>
#include <core/interface/i_backend.hpp>
#include <spectrum/pipelines/all_maxima_pipeline_rocm.hpp>

#include <hip/hip_runtime.h>

#include <vector>
#include <memory>
#include <cstdint>

// Forward declaration — keeps <core/interface/gpu_context.hpp> out of
// downstream includes (strategies_float_api is used by several callers).
namespace drv_gpu_lib { class GpuContext; }

namespace strategies {

/**
 * @class StrategiesFloatApi
 * @brief Standalone post-FFT computations from CPU float magnitudes
 *
 * Compiles strategies kernels once in the constructor via GpuContext
 * (disk-cached HSACO keyed by CompileKey).
 * Each method: H2D upload → kernel launch → D2H download (no persistent GPU state).
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
   * @param sample_rate Sampling frequency [Hz]
   * @return Vector of OneMaxParabolaLite (one per beam)
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
   * @param sample_rate Sampling frequency [Hz]
   * @return Vector of MinMaxResult (one per beam)
   */
  std::vector<MinMaxResult> GlobalMinMaxFromFloat(
      const std::vector<float>& mags,
      uint32_t n_ant, uint32_t nFFT, float sample_rate);

  /**
   * @brief Find all local maxima per beam (CPU float input)
   *
   * @param mags                Flat float magnitudes [beam_count * nFFT]
   * @param beam_count          Number of beams
   * @param nFFT                FFT size per beam
   * @param sample_rate         Sampling frequency [Hz]
   * @param dest                Output destination (CPU or GPU)
   * @param search_start        First bin to search (default 1 to skip DC)
   * @param search_end          Last bin (0 = nFFT/2)
   * @param max_maxima_per_beam Max number of maxima per beam
   * @return AllMaximaResult with beams vector
   */
  antenna_fft::AllMaximaResult AllMaximaFromMagnitudes(
      const std::vector<float>& mags,
      uint32_t beam_count, uint32_t nFFT, float sample_rate,
      antenna_fft::OutputDestination dest = antenna_fft::OutputDestination::CPU,
      uint32_t search_start = 1,
      uint32_t search_end = 0,
      uint32_t max_maxima_per_beam = 1000);

private:
  static constexpr uint32_t kBlockSize = 256;

  drv_gpu_lib::IBackend* backend_ = nullptr;
  hipStream_t            stream_  = nullptr;

  std::unique_ptr<drv_gpu_lib::GpuContext>            ctx_;
  std::unique_ptr<antenna_fft::AllMaximaPipelineROCm> all_maxima_;

  hipFunction_t minmax_kernel_  = nullptr;
  hipFunction_t one_max_kernel_ = nullptr;
};

}  // namespace strategies

#endif  // ENABLE_ROCM
