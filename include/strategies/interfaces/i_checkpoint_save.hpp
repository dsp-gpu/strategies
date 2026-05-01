#pragma once

// ============================================================================
// ICheckpointSave — Strategy для сохранения промежуточных артефактов pipeline'а
//
// ЧТО:    Pure-virtual интерфейс из 7 save_*-методов: фиксирует данные на 4
//         checkpoint-точках pipeline'а AntennaProcessor — C1 (вход: d_S/d_W),
//         C2 (после GEMM: d_X + pre/post stats), C3 (после FFT: спектр +
//         MinMaxResult), C4 (результаты сценариев: OneMaxParabolaLite).
//         Реализации: NullCheckpointSave (no-op для production) и
//         CheckpointSave (binary-файлы в Logs/GPU_XX/antenna_processor/).
//
// ЗАЧЕМ:  Без интерфейса AntennaProcessor_v1 знал бы конкретный backend
//         сохранения (файл / память / BLOB) — нарушение DIP. Через
//         ICheckpointSave* фасад зависит от абстракции: смена режима (debug
//         binary dump → production zero-cost no-op) — простая подмена
//         std::unique_ptr<ICheckpointSave> без правок самого процессора.
//         GPU-id передаётся отдельным параметром → один процессор может
//         работать на нескольких GPU без расщепления интерфейса.
//
// ПОЧЕМУ: - Null Object pattern: production-путь (NullCheckpointSave) — все
//           методы пустые inline → компилятор девиртуализует и удаляет вызовы
//           (zero overhead). Альтернатива if(ckpt) ckpt->save() в каждой
//           точке — bloat фасада, ветвление в hot path.
//         - Параметры save_* — указатель + размеры (n_ant, nFFT) + gpu_id —
//           без std::vector / std::span: данные могут быть на GPU, не нужно
//           принудительно копировать в host.
//         - 4 группы префиксов (C1/C2/C3/C4) → симметрия с pipeline-этапами,
//           легко вырезать checkpoint'ы по фазам через CheckpointSaveConfig.
//         - statistics::StatisticsResult forward — strategies НЕ зависит от
//           stats как от полного header'а. Только декларация в save_c2_stats.
//
// Использование:
//   class FileCheckpointSave : public ICheckpointSave {
//     void save_c1_signal(const void* d_data, uint32_t n_ant, uint32_t ns,
//                        float sr, int gpu_id) override {
//       /* hipMemcpy + fwrite */
//     }
//     // ... 6 остальных методов
//   };
//   auto ckpt = std::make_unique<FileCheckpointSave>();
//   processor.set_checkpoint_save(std::move(ckpt));
//
// История:
//   - Создан:  2026-03-07
//   - Изменён: 2026-05-01 (унификация формата шапки под dsp-asst RAG-индексер)
// ============================================================================

#include <strategies/result_types.hpp>

#include <cstdint>

namespace strategies {

/**
 * @class ICheckpointSave
 * @brief Strategy-интерфейс сохранения checkpoint-данных pipeline'а AntennaProcessor.
 *
 * @note Pure interface — нельзя инстанцировать. Все 7 save_*-методов обязательны.
 * @note Default реализация в production — NullCheckpointSave (no-op, zero overhead).
 * @see NullCheckpointSave — Null Object для production
 * @see AntennaProcessor_v1::set_checkpoint_save — установка стратегии
 * @see CheckpointSaveConfig — управление наборами checkpoint'ов
 */
class ICheckpointSave {
public:
  virtual ~ICheckpointSave() = default;

  /// C1: сохранить входной сигнал d_S [n_ant × n_samples] complex<float>.
  virtual void save_c1_signal(
      const void* d_data, uint32_t n_ant, uint32_t n_samples,
      float sample_rate, int gpu_id) = 0;

  /// C1: сохранить матрицу весов d_W [n_ant × n_ant] complex<float>.
  virtual void save_c1_weights(
      const void* d_weights, uint32_t n_ant, int gpu_id) = 0;

  /// C2: сохранить результат GEMM d_X [n_ant × n_samples] (после X = W·S).
  virtual void save_c2_data(
      const void* d_X, uint32_t n_ant, uint32_t n_samples,
      float sample_rate, int gpu_id) = 0;

  /// C2: сохранить пары pre/post-GEMM статистик (debug-точки 2.1 и 2.2).
  virtual void save_c2_stats(
      const statistics::StatisticsResult* pre_stats,
      const statistics::StatisticsResult* post_stats,
      uint32_t n_ant, int gpu_id) = 0;

  /// C3: сохранить полный спектр [n_ant × nFFT] complex<float> после Window+FFT.
  virtual void save_c3_spectrum(
      const void* d_spectrum, uint32_t n_ant, uint32_t nFFT, int gpu_id) = 0;

  /// C3: сохранить per-beam MinMax-результаты сценария GLOBAL_MINMAX.
  virtual void save_c3_minmax(
      const MinMaxResult* results, uint32_t n_ant, int gpu_id) = 0;

  /// C4: сохранить per-beam OneMax+Parabola-результаты сценария ONE_MAX_PARABOLA.
  virtual void save_c4_one_max(
      const OneMaxParabolaLite* results, uint32_t n_ant, int gpu_id) = 0;
};

}  // namespace strategies
