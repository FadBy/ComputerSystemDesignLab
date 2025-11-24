#include "audio_interface.h"
#include "main.h"
#include <stdio.h>

// Структура для хранения информации о текущем воспроизведении
typedef struct {
    audio_callback_t callback;
    void* user_data;
    int is_playing;
    uint32_t start_time;
    uint32_t duration;
} audio_state_t;

static audio_state_t audio_state = {0};

// Инициализация аудио интерфейса
void audio_init(void) {
    audio_state.is_playing = 0;
    audio_state.callback = NULL;
    audio_state.user_data = NULL;
    audio_state.start_time = 0;
    audio_state.duration = 0;
    
    // В реальной реализации здесь будет инициализация таймеров для ШИМ
}

// Деинициализация аудио интерфейса
void audio_deinit(void) {
    audio_stop();
}

// Функция, которая должна вызываться в основном цикле или из прерывания таймера
void audio_update(void) {
    if (audio_state.is_playing && audio_state.callback) {
        uint32_t current_time = HAL_GetTick();
        if (current_time - audio_state.start_time >= audio_state.duration) {
            // Время истекло, вызываем callback
            audio_callback_t cb = audio_state.callback;
            void* data = audio_state.user_data;
            
            // Очищаем состояние перед вызовом callback
            audio_state.is_playing = 0;
            audio_state.callback = NULL;
            audio_state.user_data = NULL;
            
            // Вызываем callback
            cb(data);
        }
    }
}

// Запуск воспроизведения ноты
void audio_start_note(uint32_t frequency, uint32_t duration, audio_callback_t callback, void* user_data) {
    // Останавливаем предыдущее воспроизведение, если есть
    audio_stop();
    
    printf("[AUDIO] Starting note: frequency=%lu Hz, duration=%lu ms\r\n", frequency, duration);
    
    // В реальной реализации здесь будет:
    // - Настройка таймера на нужную частоту (для генерации ШИМ)
    // - Включение генерации ШИМ сигнала
    // - Настройка таймера для отсчета длительности
    // - Регистрация callback для прерывания таймера
    
    // Mock: сохраняем параметры и запускаем отсчет
    audio_state.callback = callback;
    audio_state.user_data = user_data;
    audio_state.is_playing = 1;
    audio_state.start_time = HAL_GetTick();
    audio_state.duration = duration;
}

// Запуск паузы
void audio_start_pause(uint32_t duration, audio_callback_t callback, void* user_data) {
    // Останавливаем предыдущее воспроизведение, если есть
    audio_stop();
    
    printf("[AUDIO] Starting pause: duration=%lu ms\r\n", duration);
    
    // В реальной реализации здесь будет:
    // - Остановка генерации звука
    // - Настройка таймера для отсчета длительности
    // - Регистрация callback для прерывания таймера
    
    // Mock: сохраняем параметры и запускаем отсчет
    audio_state.callback = callback;
    audio_state.user_data = user_data;
    audio_state.is_playing = 1;
    audio_state.start_time = HAL_GetTick();
    audio_state.duration = duration;
}

// Остановка текущего воспроизведения
void audio_stop(void) {
    if (audio_state.is_playing) {
        audio_state.is_playing = 0;
        audio_state.callback = NULL;
        audio_state.user_data = NULL;
        printf("[AUDIO] Stopped\r\n");
        
        // В реальной реализации здесь будет остановка генерации ШИМ
    }
}

// Проверка, идет ли воспроизведение
int audio_is_playing(void) {
    return audio_state.is_playing;
}