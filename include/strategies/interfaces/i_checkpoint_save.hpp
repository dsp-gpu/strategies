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

  /**
   * @brief Checkpoint C1: фиксирует входной сигнал d_S перед GEMM.
   *
   * @param d_data GPU-указатель на complex<float>[n_ant × n_samples].
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr] }
   * @param n_ant Число антенн (строк в d_S).
   * @param n_samples Число отсчётов на антенну.
   *   @test { range=[100..1300000], value=6000 }
   * @param sample_rate Частота дискретизации, Гц.
   *   @test { range=[1.0..1e9], value=10e6, unit="Гц" }
   * @param gpu_id Идентификатор GPU (для multi-GPU).
   *   @test { range=[0..GetDeviceCount()-1], value=0 }
   */
  virtual void save_c1_signal(
      const void* d_data, uint32_t n_ant, uint32_t n_samples,
      float sample_rate, int gpu_id) = 0;

  /**
   * @brief Checkpoint C1: фиксирует матрицу весов d_W перед GEMM.
   *
   * @param d_weights GPU-указатель на complex<float>[n_ant × n_ant].
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr] }
   * @param n_ant Размер матрицы (квадратная n_ant × n_ant).
   * @param gpu_id Идентификатор GPU (для multi-GPU).
   *   @test { range=[0..GetDeviceCount()-1], value=0 }
   */
  virtual void save_c1_weights(
      const void* d_weights, uint32_t n_ant, int gpu_id) = 0;

  /**
   * @brief Checkpoint C2: фиксирует результат GEMM (X = W·S) перед FFT.
   *
   * @param d_X GPU-указатель на complex<float>[n_ant × n_samples] — результат GEMM.
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr] }
   * @param n_ant Число антенн.
   * @param n_samples Число отсчётов на антенну.
   *   @test { range=[100..1300000], value=6000 }
   * @param sample_rate Частота дискретизации, Гц.
   *   @test { range=[1.0..1e9], value=10e6, unit="Гц" }
   * @param gpu_id Идентификатор GPU.
   *   @test { range=[0..GetDeviceCount()-1], value=0 }
   */
  virtual void save_c2_data(
      const void* d_X, uint32_t n_ant, uint32_t n_samples,
      float sample_rate, int gpu_id) = 0;

  /**
   * @brief Checkpoint C2: фиксирует пары pre/post-GEMM статистик (Welford по beam'ам).
   *
   * @param pre_stats Указатель на StatisticsResult[n_ant] до GEMM (точка 2.1).
   * @param post_stats Указатель на StatisticsResult[n_ant] после GEMM (точка 2.2).
   * @param n_ant Число beam'ов в массивах.
   * @param gpu_id Идентификатор GPU.
   *   @test { range=[0..GetDeviceCount()-1], value=0 }
   */
  virtual void save_c2_stats(
      const statistics::StatisticsResult* pre_stats,
      const statistics::StatisticsResult* post_stats,
      uint32_t n_ant, int gpu_id) = 0;

  /**
   * @brief Checkpoint C3: фиксирует полный спектр после Window+FFT (до post-FFT шагов).
   *
   * @param d_spectrum GPU-указатель на complex<float>[n_ant × nFFT].
   *   @test { pattern=gpu_pointer, values=["valid_alloc", nullptr] }
   * @param n_ant Число beam'ов.
   * @param nFFT Размер FFT (степень двойки).
   *   @test { range=[8..4194304], value=1024, pattern=power_of_2 }
   * @param gpu_id Идентификатор GPU.
   *   @test { range=[0..GetDeviceCount()-1], value=0 }
   */
  virtual void save_c3_spectrum(
      const void* d_spectrum, uint32_t n_ant, uint32_t nFFT, int gpu_id) = 0;

  /**
   * @brief Checkpoint C3: фиксирует per-beam MinMax-результаты (сценарий GLOBAL_MINMAX).
   *
   * @param results Указатель на MinMaxResult[n_ant] (per-beam global min/max).
   * @param n_ant Число beam'ов в массиве.
   * @param gpu_id Идентификатор GPU.
   *   @test { range=[0..GetDeviceCount()-1], value=0 }
   */
  virtual void save_c3_minmax(
      const MinMaxResult* results, uint32_t n_ant, int gpu_id) = 0;

  /**
   * @brief Checkpoint C4: фиксирует per-beam OneMax-результаты (сценарий ONE_MAX_PARABOLA).
   *
   * @param results Указатель на OneMaxParabolaLite[n_ant] (per-beam пик с параболической интерполяцией).
   * @param n_ant Число beam'ов в массиве.
   * @param gpu_id Идентификатор GPU.
   *   @test { range=[0..GetDeviceCount()-1], value=0 }
   */
  virtual void save_c4_one_max(
      const OneMaxParabolaLite* results, uint32_t n_ant, int gpu_id) = 0;
};

}  // namespace strategies
