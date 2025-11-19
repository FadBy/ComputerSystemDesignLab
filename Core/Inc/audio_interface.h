#ifndef AUDIO_INTERFACE_H
#define AUDIO_INTERFACE_H

#include <stdint.h>

// Тип callback функции, вызываемой по истечении времени
// user_data - пользовательские данные, переданные при запуске
typedef void (*audio_callback_t)(void* user_data);

// Инициализация аудио интерфейса
void audio_init(void);

// Деинициализация аудио интерфейса
void audio_deinit(void);

// Запуск воспроизведения ноты заданной частоты и длительности
// callback вызывается по истечении duration (в мс)
// user_data передается в callback
void audio_start_note(uint32_t frequency, uint32_t duration, audio_callback_t callback, void* user_data);

// Запуск паузы заданной длительности
// callback вызывается по истечении duration (в мс)
// user_data передается в callback
void audio_start_pause(uint32_t duration, audio_callback_t callback, void* user_data);

// Остановка текущего воспроизведения (ноты или паузы)
void audio_stop(void);

// Проверка, идет ли сейчас воспроизведение
// Возвращает 1 если идет, 0 если нет
int audio_is_playing(void);

#endif // AUDIO_INTERFACE_H

