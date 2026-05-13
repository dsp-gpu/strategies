#pragma once

// ============================================================================
// AntennaProcessor_v1 — ROCm-реализация pipeline'а антенной обработки (Strategy)
//
// ЧТО:    Конкретная Strategy AntennaProcessor под ROCm. Реализует полный
//         pipeline на 7 hip-streams с overlap'ом GEMM ↔ Window+FFT ↔
//         debug-stats ↔ post-FFT сценариев:
//           1. d_S уже на GPU (caller загрузил).
//           2. Debug 2.1 — статистики на d_S (stream_debug1_).
//           3. GEMM X = W·S через hipBLAS (stream_main_).
//           4. Debug 2.2 — статистики на d_X (stream_debug2_).
//           5. Hamming-window + zero-pad + hipFFT batch → d_spectrum (stream_main_).
//           6. Debug 2.3 — статистики на |spectrum| (stream_debug3_).
//           7. Post-FFT сценарии (ONE_MAX_PARABOLA / ALL_MAXIMA / GLOBAL_MINMAX)
//              над одним d_spectrum (последовательно или 3 параллельных stream'а).
//         Поддерживает managed-weights (set_external_weights / hipFree в dtor)
//         и checkpoint-сохранение через ICheckpointSave-стратегию.
//
// ЗАЧЕМ:  Это рабочая лошадка модуля strategies — все production и большинство
//         бенчмарок гонятся через v1. AntennaProcessorTest наследуется и
//         добавляет step-by-step API для Python-валидации (NumPy/SciPy
//         референс), не меняя production-логику.
//
// ПОЧЕМУ: - Layer 6 Ref03 (Facade) — фасад потребителю, оркестратор внутри.
//           Не делает kernel-launch'ей сам, делегирует dsp::stats::
//           StatisticsProcessor / dsp::spectrum::AllMaximaPipelineROCm /
//           dsp::spectrum::ComplexToMagPhaseROCm.
//         - GpuContext ctx_ (Ref03 Layer 1) — единая точка для kernel
//           compile/cache (HSACO disk-cache по CompileKey). compiled_ flag
//           гарантирует ленивую компиляцию ровно один раз.
//         - 7 hip-stream'ов: stream_main_ + 3 debug + 3 bench3a/b/c для
//           параллельного запуска post-FFT сценариев. Overlap из 5 фаз
//           pipeline'а оптимизирует загрузку GPU (теория ROCm Cheatsheet).
//         - ScopedHipEvent (RAII) для inter-stream синхронизации —
//           event_gemm_done_, event_fft_done_, event_c1/2_done_ — нет
//           утечек hipEvent_t (правило проекта от 2026-04-15).
//         - Move/copy запрещены — owns hipBLAS handle + hipFFT plan +
//           7 streams + 7 GPU-buffers + managed weights. Lifetime
//           критичен — копирование разрушит drv_gpu_lib::IBackend контракт.
//         - protected-методы do_* — для AntennaProcessorTest: step-by-step
//           API нужно вызывать каждую фазу отдельно из Python для сравнения
//           с NumPy. Без этого пришлось бы дублировать логику.
//         - GPU-buffers — пока raw void* (миграция на BufferSet — TODO).
//           Не блокер: lifetime контролируется через allocate_buffers/
//           release_buffers симметрично.
//         - Checkpoint = ICheckpointSave* (default — NullCheckpointSave) →
//           zero overhead в production без бранчей в hot path.
//
// Использование:
//   AntennaProcessorConfig cfg{ .n_ant=16, .n_samples=8000, .sample_rate=12e6f };
//   AntennaProcessor_v1 proc(backend, cfg);
//   // (опционально) внешние веса:
//   proc.set_external_weights(W_host);
//   AntennaResult res = proc.process(d_S, proc.get_managed_weights_ptr());
//   // или с пользовательской матрицей весов на GPU:
//   AntennaResult res2 = proc.process(d_S_gpu, d_W_gpu);
//
// История:
//   - Создан:  2026-03-07
//   - Изменён: 2026-05-01 (унификация формата шапки под dsp-asst RAG-индексер)
// ============================================================================

#include <dsp/strategies/antenna_processor.hpp>
#include <dsp/strategies/interfaces/i_checkpoint_save.hpp>
#include <dsp/strategies/interfaces/i_post_fft_scenario.hpp>
#include <dsp/strategies/checkpoint/null_checkpoint_save.hpp>

#if ENABLE_ROCM
#include <core/interface/gpu_context.hpp>
#include <hip/hip_runtime.h>
#include <core/services/scoped_hip_event.hpp>
#include <hipblas/hipblas.h>
#include <hipfft/hipfft.h>
#endif

#include <memory>
#include <vector>
#include <complex>

// Forward declarations
namespace drv_gpu_lib   { class IBackend; }
namespace dsp::stats    { class StatisticsProcessor; }
namespace dsp::spectrum   { class AllMaximaPipelineROCm; }
namespace dsp::spectrum { class ComplexToMagPhaseROCm; }

namespace dsp::strategies {

/**
 * @class AntennaProcessor_v1
 * @brief ROCm-реализация AntennaProcessor: GEMM + Window+FFT + post-FFT сценарии.
 *
 * @note Move/copy запрещены — owns hipBLAS handle + hipFFT plan + 7 streams + GPU buffers.
 * @note Требует #if ENABLE_ROCM. На non-ROCm сборках большая часть кода скрыта макросом.
 * @note Lifecycle: ctor(backend, cfg) → (опц.) set_external_weights → process / step_* → dtor.
 * @note Не thread-safe. Один экземпляр = один владелец GPU-ресурсов.
 * @see AntennaProcessor — родительский Strategy-интерфейс
 * @see AntennaProcessorTest — наследник со step-by-step API для тестов
 * @see ICheckpointSave — стратегия checkpoint-сохранения (default = NullCheckpointSave)
 * @ingroup grp_strategies
 */
class AntennaProcessor_v1 : public AntennaProcessor {
public:
  explicit AntennaProcessor_v1(
      drv_gpu_lib::IBackend* backend,
      const AntennaProcessorConfig& cfg);

  ~AntennaProcessor_v1() override;

  // No copy
  AntennaProcessor_v1(const AntennaProcessor_v1&) = delete;
  AntennaProcessor_v1& operator=(const AntennaProcessor_v1&) = delete;

  // AntennaProcessor interface
  /**
   * @brief Запускает полный pipeline (Pipeline::Execute) на входе d_S/d_W; возвращает агрегированный результат.
   *
   * @param d_S Входной сигнал [n_ant × n_samples] complex<float> на GPU.
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   * @param d_W Матрица весов [n_ant × n_ant] complex<float> на GPU.
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr], error_values=[0xDEADBEEF, null] }
   *
   * @return Результат: статистики, пики (по сценарию), MinMax, метрики производительности.
   *   @test_check result.success == true
   */
  AntennaResult process(const void* d_S, const void* d_W) override;

  void set_scenario_mode(PostFftScenarioMode mode) override { cfg_.scenario_mode = mode; }
  void set_pre_input_stats(StatisticsSet stats) override { cfg_.pre_input_stats = stats; }
  void set_post_gemm_stats(StatisticsSet stats) override { cfg_.post_gemm_stats = stats; }
  void set_post_fft_stats(StatisticsSet stats) override  { cfg_.post_fft_stats  = stats; }
  void set_debug_mode(bool enabled) override { cfg_.debug_mode = enabled; }

  /**
   * @brief Возвращает текущий конфиг pipeline'а (read-only).
   *
   * @return Const-ссылка на хранимый AntennaProcessorConfig.
   *   @test_check result.n_ant > 0 && result.n_samples > 0
   */
  const AntennaProcessorConfig& config() const override { return cfg_; }
  /**
   * @brief Возвращает идентификатор GPU, на котором работает процессор.
   *
   * @return GPU id (0..GetDeviceCount()-1).
   *   @test_check result >= 0
   */
  int gpu_id() const override;

  // Checkpoint setter
  void set_checkpoint_save(std::unique_ptr<ICheckpointSave> save);

  /**
   * @brief Upload external weight matrix to GPU (managed by this class)
   * @param W Flat row-major [n_ant x n_ant] complex<float> matrix
   *
   * After this call, use get_managed_weights_ptr() to obtain the GPU pointer.
   * The buffer is freed in the destructor (not by the caller).
   */
  void set_external_weights(const std::vector<std::complex<float>>& W);

  /// GPU pointer to the last uploaded external weights (nullptr if not set)
  void* get_managed_weights_ptr() const { return d_W_managed_; }

protected:
  // Step methods for AntennaProcessorTest to call individually
  void do_debug_point_21(const void* d_S, AntennaResult& result);
  void do_gemm(const void* d_S, const void* d_W);
  void do_debug_point_22(AntennaResult& result);
  void do_window_fft();
  void do_debug_point_23(AntennaResult& result);
  void do_run_post_fft_scenarios(AntennaResult& result);
  /// 3-stream parallel variant of do_run_post_fft_scenarios (for benchmark 3.6)
  void do_run_post_fft_parallel(AntennaResult& result);

  // Access to internal buffers (for AntennaProcessorTest)
  void*    get_d_X() const { return d_X_; }
  void*    get_d_spectrum() const { return d_spectrum_; }
  void*    get_d_magnitudes() const { return d_magnitudes_; }
  uint32_t get_nFFT() const { return nFFT_; }

private:
  void allocate_buffers();
  void release_buffers();
  void ensure_compiled();
  void create_fft_plan();
  uint32_t compute_nFFT(uint32_t n_samples) const;

  // Backend
  drv_gpu_lib::IBackend* backend_ = nullptr;
  AntennaProcessorConfig cfg_;

#if ENABLE_ROCM
  // Ref03: GpuContext for kernel compilation (replaces manual hiprtc + KernelCacheService)
  drv_gpu_lib::GpuContext ctx_;
  bool compiled_ = false;

  // HIP streams (7 for multi-stream pipeline)
  hipStream_t stream_main_    = nullptr;
  hipStream_t stream_debug1_  = nullptr;
  hipStream_t stream_debug2_  = nullptr;
  hipStream_t stream_debug3_  = nullptr;
  hipStream_t stream_bench3a_ = nullptr;
  hipStream_t stream_bench3b_ = nullptr;
  hipStream_t stream_bench3c_ = nullptr;

  // HIP events (inter-stream sync)
  drv_gpu_lib::ScopedHipEvent event_gemm_done_;
  drv_gpu_lib::ScopedHipEvent event_fft_done_;
  drv_gpu_lib::ScopedHipEvent event_c1_done_;
  drv_gpu_lib::ScopedHipEvent event_c2_done_;

  // hipBLAS
  hipblasHandle_t hipblas_handle_ = nullptr;

  // hipFFT
  hipfftHandle fft_plan_ = 0;
  bool fft_plan_created_ = false;

  // GPU buffers (raw — migrated to BufferSet later)
  void* d_X_          = nullptr;
  void* d_fft_input_  = nullptr;
  void* d_spectrum_   = nullptr;
  void* d_magnitudes_ = nullptr;
  void* d_hamming_window_ = nullptr;
  void* d_one_max_results_ = nullptr;
  void* d_minmax_results_  = nullptr;
#endif

  // Externally supplied weight matrix (managed GPU buffer — freed by release_buffers)
  void* d_W_managed_ = nullptr;

  // Sizes
  uint32_t nFFT_ = 0;

  // Components
  std::unique_ptr<dsp::stats::StatisticsProcessor> stats_processor_;
  std::unique_ptr<dsp::spectrum::AllMaximaPipelineROCm> all_maxima_pipeline_;
  std::unique_ptr<dsp::spectrum::ComplexToMagPhaseROCm> complex_to_mag_;
  std::unique_ptr<ICheckpointSave> checkpoint_;

  static constexpr uint32_t kBlockSize = 256;
};

} // namespace dsp::strategies
