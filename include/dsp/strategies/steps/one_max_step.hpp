#pragma once

// ============================================================================
// OneMaxStep — один глобальный максимум на луч с параболикой (Ref03-C Step)
//
// ЧТО:    Pipeline-шаг сценария ALL_REQUIRED / ONE_MAX_PARABOLA. Запускает
//         HIP-kernel one_max_no_phase (grid=(1, n_ant, 1), 256 threads/block):
//         каждый y-блок ищет глобальный max в kBufMagnitudes по своему лучу,
//         читает 3 точки kBufSpectrum вокруг пика и делает 3-точечную
//         параболическую интерполяцию для refined-частоты с точностью
//         в ~1/100 bin'а. Результат [n_ant × OneMaxParabolaLite] →
//         D2H → result->one_max.
//
// ЗАЧЕМ:  Когда нужен ровно ОДИН доминирующий пик на луч (типичный
//         сценарий FMCW-радара: один tone от одной цели), AllMaximaStep
//         избыточен — он ищет все локальные максимумы, фильтрует threshold,
//         сортирует. OneMaxStep делает это в один pass без сортировки и
//         в ~3-5× быстрее на типичных размерах nFFT=4096..16384.
//
// ПОЧЕМУ: - one_max_no_phase (не one_max_with_phase) — lite-версия БЕЗ
//           извлечения фазы пика. Фаза не нужна для оценки доплера/дальности,
//           зато структура результата OneMaxParabolaLite в ~2× меньше
//           (без поля phase) → меньше D2H и меньше Python-overhead.
//         - Параболическая интерполяция по 3 точкам |X[k-1]|, |X[k]|,
//           |X[k+1]|: delta = 0.5 × (|X[k-1]| - |X[k+1]|) / (|X[k-1]| - 2|X[k]| + |X[k+1]|).
//           Дешёво (5 операций), даёт refined freq с точностью ~1/100 bin'а
//           для синусоиды — стандартный приём DSP.
//         - SetStream(s) — для PARALLEL-группы вместе с AllMaximaStep
//           и MinMaxStep на разных streams (stream_post_a/b/c).
//         - hipStreamSynchronize ПЕРЕД memcpy — те же причины что в
//           MinMaxStep (без sync — race на D2H).
//
// Использование:
//   auto step = std::make_unique<OneMaxStep>();
//   step->SetStream(ctx.stream_post_a);
//   builder.add(std::move(step));
//
// История:
//   - Создан: 2026-03-14 (Ref03-C, выделено из AntennaProcessor)
// ============================================================================


#include <dsp/strategies/i_pipeline_step.hpp>
#include <dsp/strategies/pipeline_context.hpp>

#include <hip/hip_runtime.h>
#include <stdexcept>

namespace dsp::strategies {

/**
 * @class OneMaxStep
 * @brief Pipeline-шаг: один глобальный max на луч + 3-точечная параболическая интерполяция.
 *
 * @note Lite-версия (без фазы) — структура OneMaxParabolaLite в 2× меньше OneMaxParabola.
 * @note SetStream(s) — для PARALLEL-группы (по умолчанию stream_debug3).
 * @see MinMaxStep    — глобальные min+max за один проход.
 * @see AllMaximaStep — все локальные максимумы (избыточно для одного пика).
 */
class OneMaxStep : public PipelineStepBase {
public:
  /**
   * @brief Возвращает имя шага для логирования и поиска через Pipeline::FindStep.
   *
   * @return C-строка "OneMaxParabola" (статический литерал).
   *   @test_check std::string(result) == "OneMaxParabola"
   */
  const char* Name() const override { return "OneMaxParabola"; }

  /**
   * @brief Активен в сценариях ALL_REQUIRED и ONE_MAX_PARABOLA (поиск одного максимума с параболической интерполяцией).
   *
   * @param cfg Конфиг pipeline'а (scenario_mode определяет активность).
   *   @test_ref AntennaProcessorConfig
   *
   * @return true для ALL_REQUIRED / ONE_MAX_PARABOLA, иначе false.
   *   @test_check result == (cfg.scenario_mode == PostFftScenarioMode::ALL_REQUIRED || cfg.scenario_mode == PostFftScenarioMode::ONE_MAX_PARABOLA)
   */
  bool IsEnabled(const AntennaProcessorConfig& cfg) const override {
    return cfg.scenario_mode == PostFftScenarioMode::ALL_REQUIRED ||
           cfg.scenario_mode == PostFftScenarioMode::ONE_MAX_PARABOLA;
  }

  /// Назначить stream (для PARALLEL-группы); default = ctx.stream_debug3.
  void SetStream(hipStream_t s) { override_stream_ = s; }

  /**
   * @brief Запустить one_max_no_phase kernel, синхронизировать stream, D2H в result->one_max.
   * @param ctx Shared context: gpu_ctx, kBufMagnitudes, kBufSpectrum, kBufOneMaxResults, n_ant, nFFT, sample_rate.
   *   @test { values=["valid_backend"] }
   * @throws std::runtime_error если hipModuleLaunchKernel вернул не hipSuccess.
   *   @test_check throws on hipModuleLaunchKernel != hipSuccess
   */
  void Execute(PipelineContext& ctx) override {
    uint32_t n_ant = ctx.cfg->n_ant;
    float sr = ctx.cfg->sample_rate;
    hipStream_t stream = override_stream_ ? override_stream_ : ctx.stream_debug3;

    void* d_mag = ctx.buf(kBufMagnitudes);
    void* d_spec = ctx.buf(kBufSpectrum);
    void* d_results = ctx.buf(kBufOneMaxResults);

    void* args[] = { &d_mag, &d_spec, &d_results, &n_ant, &ctx.nFFT, &sr };

    hipError_t err = hipModuleLaunchKernel(
        ctx.gpu_ctx->GetKernel("one_max_no_phase"),
        1, n_ant, 1,
        kBlockSize, 1, 1,
        0, stream,
        args, nullptr);
    if (err != hipSuccess) {
      throw std::runtime_error("OneMaxStep: kernel launch failed");
    }

    // Sync + D2H copy
    hipStreamSynchronize(stream);
    ctx.result->one_max.resize(n_ant);
    hipMemcpy(ctx.result->one_max.data(), d_results,
              n_ant * sizeof(OneMaxParabolaLite), hipMemcpyDeviceToHost);
  }

private:
  hipStream_t override_stream_ = nullptr;
  static constexpr unsigned int kBlockSize = 256;
};

} // namespace dsp::strategies

