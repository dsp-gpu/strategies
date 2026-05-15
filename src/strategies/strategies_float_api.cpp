/**
 * @file strategies_float_api.cpp
 * @brief Implementation for StrategiesFloatApi — CPU-vector post-FFT wrappers.
 *
 * Phase C4 of kernel_cache_v2:
 *   - Split from 337-line header-inline.
 *   - Kernel compilation delegated to GpuContext (clean-slate v2 disk cache).
 *   - Manual hiprtc + KernelCacheService removed (-70 LOC boilerplate).
 *
 * @date 2026-04-22
 */


#include <dsp/strategies/strategies_float_api.hpp>
#include <dsp/strategies/kernels/strategies_kernels_rocm.hpp>

#include <core/backends/rocm/rocm_backend.hpp>
#include <core/interface/gpu_context.hpp>
#include <core/services/cache_dir_resolver.hpp>

#include <stdexcept>
#include <string>

namespace dsp::strategies {

// ============================================================================
// Ctor / Dtor
// ============================================================================

StrategiesFloatApi::StrategiesFloatApi(drv_gpu_lib::IBackend* backend)
    : backend_(backend)
{
  if (!backend_ || !backend_->IsInitialized())
    throw std::runtime_error("StrategiesFloatApi: backend is null or not initialized");

  if (backend_->GetType() != drv_gpu_lib::BackendType::ROCm)
    throw std::runtime_error("StrategiesFloatApi: requires ROCm backend");

  stream_ = static_cast<hipStream_t>(backend_->GetNativeQueue());
  if (!stream_)
    throw std::runtime_error("StrategiesFloatApi: failed to get HIP stream");

  all_maxima_ = std::make_unique<::antenna_fft::AllMaximaPipelineROCm>(stream_, backend_);

  // GpuContext v2 — compile + disk cache via CompileKey.
  ctx_ = std::make_unique<drv_gpu_lib::GpuContext>(
      backend_, "Strategies",
      drv_gpu_lib::ResolveCacheDir("strategies"));

  // WARP_SIZE влияет на бинарник → попадает в CompileKey → gfx908 и gfx1201
  // получают независимые HSACO файлы на диске.
  int warp_size = 32;
  try {
    auto* rb = static_cast<drv_gpu_lib::ROCmBackend*>(backend_);
    warp_size = rb->GetCore().GetWarpSize();
  } catch (...) {}

  std::vector<std::string> defines{
      "-DWARP_SIZE=" + std::to_string(warp_size),
      "-DBLOCK_SIZE=256",
  };

  const char* src = kernels::GetStrategiesHIPKernelSource();
  ctx_->CompileModule(src,
                      {"global_minmax", "one_max_no_phase"},
                      defines);

  minmax_kernel_  = ctx_->GetKernel("global_minmax");
  one_max_kernel_ = ctx_->GetKernel("one_max_no_phase");
}

StrategiesFloatApi::~StrategiesFloatApi() = default;

// ============================================================================
// OneMaxParabolaFromFloat
// ============================================================================

std::vector<OneMaxParabolaLite> StrategiesFloatApi::OneMaxParabolaFromFloat(
    const std::vector<float>& mags,
    uint32_t n_ant, uint32_t nFFT, float sample_rate)
{
  if (mags.size() != static_cast<size_t>(n_ant) * nFFT)
    throw std::invalid_argument("OneMaxParabolaFromFloat: mags.size() != n_ant * nFFT");

  const size_t mag_bytes    = mags.size() * sizeof(float);
  const size_t zeros_bytes  = static_cast<size_t>(n_ant) * nFFT * sizeof(float) * 2;  // float2
  const size_t result_bytes = n_ant * sizeof(OneMaxParabolaLite);

  void* d_mag    = nullptr;
  void* d_zeros  = nullptr;
  void* d_result = nullptr;

  if (hipMalloc(&d_mag,    mag_bytes)    != hipSuccess ||
      hipMalloc(&d_zeros,  zeros_bytes)  != hipSuccess ||
      hipMalloc(&d_result, result_bytes) != hipSuccess)
  {
    hipFree(d_mag); hipFree(d_zeros); hipFree(d_result);
    throw std::runtime_error("OneMaxParabolaFromFloat: hipMalloc failed");
  }

  hipMemcpy(d_mag, mags.data(), mag_bytes, hipMemcpyHostToDevice);
  hipMemset(d_zeros, 0, zeros_bytes);

  // one_max_no_phase kernel: args = (d_magnitudes, d_spectrum, d_results, n_ant, nFFT, sample_rate)
  void* args[] = { &d_mag, &d_zeros, &d_result, &n_ant, &nFFT, &sample_rate };
  hipModuleLaunchKernel(
      one_max_kernel_,
      1, n_ant, 1,
      kBlockSize, 1, 1,
      0, stream_,
      args, nullptr);

  hipStreamSynchronize(stream_);

  std::vector<OneMaxParabolaLite> out(n_ant);
  hipMemcpy(out.data(), d_result, result_bytes, hipMemcpyDeviceToHost);

  hipFree(d_mag);
  hipFree(d_zeros);
  hipFree(d_result);

  return out;
}

// ============================================================================
// GlobalMinMaxFromFloat
// ============================================================================

std::vector<MinMaxResult> StrategiesFloatApi::GlobalMinMaxFromFloat(
    const std::vector<float>& mags,
    uint32_t n_ant, uint32_t nFFT, float sample_rate)
{
  if (mags.size() != static_cast<size_t>(n_ant) * nFFT)
    throw std::invalid_argument("GlobalMinMaxFromFloat: mags.size() != n_ant * nFFT");

  const size_t mag_bytes    = mags.size() * sizeof(float);
  const size_t result_bytes = n_ant * sizeof(MinMaxResult);

  void* d_mag    = nullptr;
  void* d_result = nullptr;

  if (hipMalloc(&d_mag,    mag_bytes)    != hipSuccess ||
      hipMalloc(&d_result, result_bytes) != hipSuccess)
  {
    hipFree(d_mag); hipFree(d_result);
    throw std::runtime_error("GlobalMinMaxFromFloat: hipMalloc failed");
  }

  hipMemcpy(d_mag, mags.data(), mag_bytes, hipMemcpyHostToDevice);

  // global_minmax kernel: args = (d_magnitudes, d_results, n_ant, nFFT, sample_rate)
  void* args[] = { &d_mag, &d_result, &n_ant, &nFFT, &sample_rate };
  hipModuleLaunchKernel(
      minmax_kernel_,
      1, n_ant, 1,
      kBlockSize, 1, 1,
      0, stream_,
      args, nullptr);

  hipStreamSynchronize(stream_);

  std::vector<MinMaxResult> out(n_ant);
  hipMemcpy(out.data(), d_result, result_bytes, hipMemcpyDeviceToHost);

  hipFree(d_mag);
  hipFree(d_result);

  return out;
}

// ============================================================================
// AllMaximaFromMagnitudes
// ============================================================================

::antenna_fft::AllMaximaResult StrategiesFloatApi::AllMaximaFromMagnitudes(
    const std::vector<float>& mags,
    uint32_t beam_count, uint32_t nFFT, float sample_rate,
    ::drv_gpu_lib::OutputDestination dest,
    uint32_t search_start,
    uint32_t search_end,
    uint32_t max_maxima_per_beam)
{
  if (mags.size() != static_cast<size_t>(beam_count) * nFFT)
    throw std::invalid_argument("AllMaximaFromMagnitudes: mags.size() != beam_count * nFFT");

  const size_t mag_bytes   = mags.size() * sizeof(float);
  const size_t zeros_bytes = static_cast<size_t>(beam_count) * nFFT * sizeof(float) * 2;  // float2

  void* d_mag   = nullptr;
  void* d_zeros = nullptr;

  if (hipMalloc(&d_mag,   mag_bytes)   != hipSuccess ||
      hipMalloc(&d_zeros, zeros_bytes) != hipSuccess)
  {
    hipFree(d_mag); hipFree(d_zeros);
    throw std::runtime_error("AllMaximaFromMagnitudes: hipMalloc failed");
  }

  hipMemcpy(d_mag, mags.data(), mag_bytes, hipMemcpyHostToDevice);
  hipMemset(d_zeros, 0, zeros_bytes);

  auto result = all_maxima_->Execute(
      d_mag,
      d_zeros,
      beam_count,
      nFFT,
      sample_rate,
      dest,
      search_start,
      search_end,
      max_maxima_per_beam);

  hipFree(d_mag);
  hipFree(d_zeros);

  return result;
}

} // namespace dsp::strategies

