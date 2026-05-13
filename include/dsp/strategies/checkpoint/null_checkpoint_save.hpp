#pragma once

// ============================================================================
// NullCheckpointSave — Null Object реализация ICheckpointSave (production default)
//
// ЧТО:    Пустая реализация ICheckpointSave: все 7 save_*-методов — inline
//         no-op'ы (`{}`). final-класс. Подставляется по умолчанию в
//         AntennaProcessor_v1, когда чекпоинт-сохранение не нужно
//         (production-путь без debug-дампов).
//
// ЗАЧЕМ:  AntennaProcessor_v1 в hot-path вызывает 7 точек сохранения
//         (C1/C2/C3/C4). В production эти данные не нужны, но условные
//         проверки `if (ckpt) ckpt->save()` в каждой точке — bloat и
//         branch в hot loop. Null Object убирает if'ы: фасад всегда
//         вызывает ckpt_->save_*(), а в production — это пустая inline-
//         функция, которую компилятор девиртуализует и удаляет (LTO).
//
// ПОЧЕМУ: - Null Object pattern (GoF) → unconditional calls вместо
//           nullptr-check'ов. Чище код фасада, нет ветвления в hot path.
//         - `final` на классе → компилятор знает, что vtable не подменится,
//           может devirtualize-вать вызовы (zero overhead на практике).
//         - inline пустые тела в .hpp → нет .cpp, нет линковки, всё
//           разворачивается на месте.
//         - Реализация в strategies/checkpoint/ → дисциплина: интерфейс в
//           interfaces/, реализации в checkpoint/. Будущие FileCheckpointSave
//           / MemCheckpointSave лягут в ту же папку.
//
// Использование:
//   // По умолчанию AntennaProcessor_v1 ставит NullCheckpointSave сам.
//   // Явное переключение в production:
//   processor.set_checkpoint_save(std::make_unique<NullCheckpointSave>());
//   // Debug-сборка:
//   processor.set_checkpoint_save(std::make_unique<FileCheckpointSave>(path));
//
// История:
//   - Создан:  2026-03-07
//   - Изменён: 2026-05-01 (унификация формата шапки под dsp-asst RAG-индексер)
// ============================================================================

#include <dsp/strategies/interfaces/i_checkpoint_save.hpp>

namespace dsp::strategies {

/**
 * @class NullCheckpointSave
 * @brief Null Object для ICheckpointSave — все save_*-методы пустые inline no-op'ы.
 *
 * @note final → компилятор может devirtualize-вать вызовы (zero overhead).
 * @note Используется по умолчанию AntennaProcessor_v1, когда чекпоинт не задан.
 * @see ICheckpointSave — родительский интерфейс
 * @see AntennaProcessor_v1::set_checkpoint_save
 */
class NullCheckpointSave final : public ICheckpointSave {
public:
  /**
   * @brief Null Object: no-op (zero overhead — devirtualized + inline пустое тело).
   *
   */
  void save_c1_signal(const void*, uint32_t, uint32_t, float, int) override {}
  /**
   * @brief Null Object: no-op (zero overhead — devirtualized + inline пустое тело).
   *
   */
  void save_c1_weights(const void*, uint32_t, int) override {}
  /**
   * @brief Null Object: no-op (zero overhead — devirtualized + inline пустое тело).
   *
   */
  void save_c2_data(const void*, uint32_t, uint32_t, float, int) override {}
  /**
   * @brief Null Object: no-op (zero overhead — devirtualized + inline пустое тело).
   *
   */
  void save_c2_stats(const dsp::stats::StatisticsResult*, const dsp::stats::StatisticsResult*,
                     uint32_t, int) override {}
  /**
   * @brief Null Object: no-op (zero overhead — devirtualized + inline пустое тело).
   *
   */
  void save_c3_spectrum(const void*, uint32_t, uint32_t, int) override {}
  /**
   * @brief Null Object: no-op (zero overhead — devirtualized + inline пустое тело).
   *
   */
  void save_c3_minmax(const MinMaxResult*, uint32_t, int) override {}
  /**
   * @brief Null Object: no-op (zero overhead — devirtualized + inline пустое тело).
   *
   */
  void save_c4_one_max(const OneMaxParabolaLite*, uint32_t, int) override {}
};

} // namespace dsp::strategies
