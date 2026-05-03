#pragma once

// ============================================================================
// IPostFftScenario — Strategy для post-FFT обработки одного спектра
//
// ЧТО:    Pure-virtual интерфейс post-FFT сценария: один метод execute() +
//         name(). Реализации (OneMaxParabolaScenario, AllMaximaScenario,
//         GlobalMinMaxScenario) живут в модуле spectrum/ и работают над
//         одним и тем же d_spectrum, посчитанным один раз блоком Window+FFT
//         в AntennaProcessor_v1.
//
// ЗАЧЕМ:  3 разных post-FFT задачи (один максимум + парабола / все максимумы /
//         глобальный MinMax) поверх одного буфера спектра. Без интерфейса
//         AntennaProcessor либо хардкодит 3 ветки if/else, либо знает
//         конкретные классы — нарушение OCP/DIP. Через IPostFftScenario*
//         можно: запустить любой набор сценариев, добавить новый сценарий
//         без правки фасада, прокинуть unit-test mock через тот же API.
//
// ПОЧЕМУ: - Один execute() с фиксированной сигнатурой → ISP не нарушается:
//           всем сценариям нужны те же 7 параметров (spectrum, magnitudes,
//           размеры, sample_rate, maxima_limit, stream). Дополнительные
//           buffer'ы — приватные у конкретной реализации.
//         - d_magnitudes отдельным параметром (может быть nullptr) —
//           сценарии типа OneMaxParabola требуют |spectrum|, GlobalMinMax
//           тоже, а потенциальный «raw spectrum analyzer» — нет. nullptr
//           = «не считай заранее» (передача отв-ти реализации).
//         - hipStream_t в API → caller контролирует параллельность:
//           AntennaProcessor_v1 запускает 3 сценария на 3 разных streams
//           (do_run_post_fft_parallel) для overlap.
//         - #if ENABLE_ROCM на execute() — non-ROCm сборка получает только
//           name() (для тестов компиляции). Без HIP-зависимостей в API.
//         - name() const char* — лёгкий для логов/профайлера, без
//           аллокаций std::string.
//
// Использование:
//   class GlobalMinMaxScenario : public IPostFftScenario {
//     void execute(const void* d_spec, const void* d_mag, uint32_t n_ant,
//                 uint32_t nFFT, float sr, uint32_t /*lim*/,
//                 hipStream_t s) override {
//       // launch global_minmax kernel on stream s
//     }
//     const char* name() const override { return "GlobalMinMax"; }
//   };
//
// История:
//   - Создан:  2026-03-07
//   - Изменён: 2026-05-01 (унификация формата шапки под dsp-asst RAG-индексер)
// ============================================================================

#include <strategies/result_types.hpp>

#include <cstdint>

#if ENABLE_ROCM
#include <hip/hip_runtime.h>
#endif

namespace strategies {

/**
 * @class IPostFftScenario
 * @brief Strategy-интерфейс post-FFT сценария над одним предсчитанным d_spectrum.
 *
 * @note Pure interface — нельзя инстанцировать. На non-ROCm сборках доступен только name().
 * @note d_magnitudes может быть nullptr (реализация сама решает, считать ли |spectrum|).
 * @see AntennaProcessor_v1::do_run_post_fft_scenarios — последовательный запуск.
 * @see AntennaProcessor_v1::do_run_post_fft_parallel — параллельный запуск (3 stream).
 * @see PostFftScenarioMode — селектор активных сценариев в AntennaProcessorConfig.
 */
class IPostFftScenario {
public:
  virtual ~IPostFftScenario() = default;

#if ENABLE_ROCM
  /**
   * @brief Запустить post-FFT сценарий над уже посчитанным спектром.
   * @param d_spectrum   Complex-float спектр [n_ant × nFFT] на GPU.
   * @param d_magnitudes Float-модули |spectrum| [n_ant × nFFT] на GPU (или nullptr).
   * @param n_ant        Количество лучей / антенн.
   * @param nFFT         Размер FFT (после nextPow2 + zero-padding).
   * @param sample_rate  Частота дискретизации, Гц (для freq[k] = k·sr/nFFT).
   * @param maxima_limit Лимит пиков для AllMaxima (игнорируется другими сценариями).
   * @param stream       HIP stream для асинхронного исполнения.
   */
  virtual void execute(
      const void* d_spectrum,
      const void* d_magnitudes,
      uint32_t n_ant,
      uint32_t nFFT,
      float    sample_rate,
      uint32_t maxima_limit,
      hipStream_t stream) = 0;
#endif

  /**
   * @brief Возвращает человекочитаемое имя сценария (для профайлера и логов).
   *
   * @return C-строка имени (например, "OneMaxParabola" / "AllMaxima" / "GlobalMinMax").
   *   @test_check result != nullptr && std::strlen(result) > 0
   */
  virtual const char* name() const = 0;
};

}  // namespace strategies
