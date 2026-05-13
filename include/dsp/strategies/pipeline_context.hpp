#pragma once

/**
 * @file pipeline_context.hpp
 * @brief PipelineContext — shared state-агрегатор для всех IPipelineStep + enum индексов буферов.
 *
 * @note Тип B (technical header): композиционный non-owning struct + enum
 *       PipelineBuf. Без логики кроме одного inline-accessor buf(id).
 *       Владение полей — у Facade (AntennaProcessor_v1): backend, gpu_ctx,
 *       cfg, GPU handles (hipblas/hipfft), streams/events, BufferSet,
 *       sub-processors. Поля immutable после конструктора Facade'а;
 *       d_S/d_W/result обновляются перед каждым process()/Pipeline::Execute.
 *
 * История:
 *   - Создан:  2026-03-14 (Ref03-C, выделено из AntennaProcessor)
 *   - Изменён: 2026-05-01 (унификация формата шапки под dsp-asst RAG-индексер)
 */

#if ENABLE_ROCM

#include <dsp/strategies/config/antenna_processor_config.hpp>
#include <dsp/strategies/result_types.hpp>
#include <dsp/strategies/interfaces/i_checkpoint_save.hpp>
#include <core/services/buffer_set.hpp>

#include <hip/hip_runtime.h>
#include <hipblas/hipblas.h>
#include <hipfft/hipfft.h>

// Forward declarations
namespace drv_gpu_lib   { class IBackend; class GpuContext; }
namespace dsp::stats    { class StatisticsProcessor; }
namespace dsp::spectrum   { class AllMaximaPipelineROCm; }
namespace dsp::spectrum { class ComplexToMagPhaseROCm; }

namespace dsp::strategies {

/**
 * @enum PipelineBuf
 * @brief Индексы в BufferSet<kBufCount>; используются всеми шагами через ctx.buf(id).
 * @note Compile-time индексы (size_t) — ноль runtime-overhead; добавление
 *       нового буфера = вставка значения ДО kBufCount + увеличение размера.
 */
enum PipelineBuf : size_t {
  kBufX = 0,            ///< [n_ant × n_samples] GEMM output
  kBufFftInput,         ///< [n_ant × nFFT] zero-padded for FFT
  kBufSpectrum,         ///< [n_ant × nFFT] FFT output (complex)
  kBufMagnitudes,       ///< [n_ant × nFFT] float |spectrum|
  kBufHammingWindow,    ///< [n_samples] precomputed Hamming window
  kBufOneMaxResults,    ///< [n_ant] OneMaxParabolaLite
  kBufMinMaxResults,    ///< [n_ant] MinMaxResult
  kBufCount
};

/**
 * @struct PipelineContext
 * @brief Shared non-owning агрегатор: backend, kernels, streams, buffers, sub-processors, result.
 *
 * @note Все указатели non-owning (владелец — Facade AntennaProcessor_v1).
 * @note Заполняется один раз в конструкторе фасада; d_S/d_W/result —
 *       перед каждым Pipeline::Execute (это per-call inputs/outputs).
 * @see Pipeline           — потребитель: каждый Entry получает ссылку.
 * @see IPipelineStep      — Execute(PipelineContext&) — единственный способ доступа к ресурсам.
 * @see drv_gpu_lib::BufferSet — управление GPU-буферами через enum PipelineBuf.
 */
struct PipelineContext {
  // ── Immutable references (set once in constructor) ────────────────────
  drv_gpu_lib::IBackend* backend = nullptr;
  drv_gpu_lib::GpuContext* gpu_ctx = nullptr;
  const AntennaProcessorConfig* cfg = nullptr;

  // ── GPU handles (owned by Facade) ─────────────────────────────────────
  hipblasHandle_t hipblas_handle = nullptr;
  hipfftHandle    fft_plan = 0;

  // Streams
  hipStream_t stream_main   = nullptr;   ///< GEMM + Window+FFT
  hipStream_t stream_debug1 = nullptr;   ///< Debug 2.1 (stats on d_S)
  hipStream_t stream_debug2 = nullptr;   ///< Debug 2.2 (stats on d_X)
  hipStream_t stream_debug3 = nullptr;   ///< Debug 2.3 + post-FFT sequential
  hipStream_t stream_post_a = nullptr;   ///< Parallel post-FFT: OneMax
  hipStream_t stream_post_b = nullptr;   ///< Parallel post-FFT: AllMaxima
  hipStream_t stream_post_c = nullptr;   ///< Parallel post-FFT: MinMax

  // Events (inter-stream synchronization)
  hipEvent_t event_gemm_done = nullptr;
  hipEvent_t event_fft_done  = nullptr;
  hipEvent_t event_c1_done   = nullptr;
  hipEvent_t event_c2_done   = nullptr;

  // ── GPU buffers (owned by Facade via BufferSet) ───────────────────────
  drv_gpu_lib::BufferSet<kBufCount>* buffers = nullptr;

  // ── Sizes ─────────────────────────────────────────────────────────────
  uint32_t nFFT = 0;
  int gpu_id = 0;

  // ── Sub-processors (owned by Facade, non-owning pointers) ─────────────
  dsp::stats::StatisticsProcessor*      stats_processor = nullptr;
  dsp::spectrum::AllMaximaPipelineROCm*   all_maxima_pipeline = nullptr;
  dsp::spectrum::ComplexToMagPhaseROCm* complex_to_mag = nullptr;
  ICheckpointSave*                      checkpoint = nullptr;

  // ── Per-call inputs (set before Pipeline::Execute) ────────────────────
  const void* d_S = nullptr;   ///< Input signal (GPU)
  const void* d_W = nullptr;   ///< Weight matrix (GPU)

  // ── Accumulated result (steps write into this) ────────────────────────
  AntennaResult* result = nullptr;

  // ── Helper accessors ──────────────────────────────────────────────────
  /**
   * @brief Возвращает device-указатель буфера по enum-индексу (через BufferSet::Get).
   *
   * @param id Enum-индекс буфера (kBufX / kBufSpectrum / kBufMagnitudes / ...).
   */
  void* buf(PipelineBuf id) const {
    return buffers->Get(static_cast<size_t>(id));
  }
};

} // namespace dsp::strategies

#endif  // ENABLE_ROCM
