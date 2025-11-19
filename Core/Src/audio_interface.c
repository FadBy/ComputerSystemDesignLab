#define _POSIX_C_SOURCE 200809L
#include "audio_interface.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>

// Mock-реализация для тестирования без контроллера
// В реальной реализации здесь будет работа с таймером STM32

// Структура для хранения информации о текущем воспроизведении
typedef struct {
    audio_callback_t callback;
    void* user_data;
    pthread_t thread_id;
    int is_playing;
    int should_stop;
} audio_state_t;

static audio_state_t audio_state = {0};

// Функция потока для задержки и вызова callback
static void* delay_thread(void* arg) {
    uint32_t duration = *(uint32_t*)arg;
    free(arg);
    
    // Задержка
    struct timespec ts;
    ts.tv_sec = duration / 1000;
    ts.tv_nsec = (duration % 1000) * 1000000;
    nanosleep(&ts, NULL);
    
    // Проверяем, не была ли остановлена
    if (!audio_state.should_stop && audio_state.callback) {
        audio_callback_t cb = audio_state.callback;
        void* data = audio_state.user_data;
        
        // Очищаем состояние перед вызовом callback
        audio_state.is_playing = 0;
        audio_state.callback = NULL;
        audio_state.user_data = NULL;
        
        // Вызываем callback
        cb(data);
    } else {
        audio_state.is_playing = 0;
    }
    
    return NULL;
}

// Инициализация аудио интерфейса
void audio_init(void) {
    audio_state.is_playing = 0;
    audio_state.should_stop = 0;
    audio_state.callback = NULL;
    audio_state.user_data = NULL;
}

// Деинициализация аудио интерфейса
void audio_deinit(void) {
    audio_stop();
}

// Запуск воспроизведения ноты
void audio_start_note(uint32_t frequency, uint32_t duration, audio_callback_t callback, void* user_data) {
    // Останавливаем предыдущее воспроизведение, если есть
    audio_stop();
    
    printf("[AUDIO] Starting note: frequency=%u Hz, duration=%u ms\n", frequency, duration);
    
    // В реальной реализации здесь будет:
    // - Настройка таймера на нужную частоту (для генерации ШИМ)
    // - Включение генерации ШИМ сигнала
    // - Настройка таймера для отсчета длительности
    // - Регистрация callback для прерывания таймера
    
    // Mock: запускаем поток с задержкой
    audio_state.callback = callback;
    audio_state.user_data = user_data;
    audio_state.is_playing = 1;
    audio_state.should_stop = 0;
    
    // Передаем duration в поток
    uint32_t* duration_ptr = (uint32_t*)malloc(sizeof(uint32_t));
    if (!duration_ptr) {
        printf("[AUDIO] Error: Failed to allocate memory\n");
        audio_state.is_playing = 0;
        return;
    }
    *duration_ptr = duration;
    
    if (pthread_create(&audio_state.thread_id, NULL, delay_thread, duration_ptr) != 0) {
        printf("[AUDIO] Error: Failed to create thread\n");
        free(duration_ptr);
        audio_state.is_playing = 0;
    }
}

// Запуск паузы
void audio_start_pause(uint32_t duration, audio_callback_t callback, void* user_data) {
    // Останавливаем предыдущее воспроизведение, если есть
    audio_stop();
    
    printf("[AUDIO] Starting pause: duration=%u ms\n", duration);
    
    // В реальной реализации здесь будет:
    // - Остановка генерации звука
    // - Настройка таймера для отсчета длительности
    // - Регистрация callback для прерывания таймера
    
    // Mock: запускаем поток с задержкой
    audio_state.callback = callback;
    audio_state.user_data = user_data;
    audio_state.is_playing = 1;
    audio_state.should_stop = 0;
    
    // Передаем duration в поток
    uint32_t* duration_ptr = (uint32_t*)malloc(sizeof(uint32_t));
    if (!duration_ptr) {
        printf("[AUDIO] Error: Failed to allocate memory\n");
        audio_state.is_playing = 0;
        return;
    }
    *duration_ptr = duration;
    
    if (pthread_create(&audio_state.thread_id, NULL, delay_thread, duration_ptr) != 0) {
        printf("[AUDIO] Error: Failed to create thread\n");
        free(duration_ptr);
        audio_state.is_playing = 0;
    }
}

// Остановка текущего воспроизведения
void audio_stop(void) {
    if (audio_state.is_playing) {
        audio_state.should_stop = 1;
        // Ждем завершения потока (с таймаутом)
        pthread_join(audio_state.thread_id, NULL);
        audio_state.is_playing = 0;
        audio_state.should_stop = 0;
        audio_state.callback = NULL;
        audio_state.user_data = NULL;
        printf("[AUDIO] Stopped\n");
    }
}

// Проверка, идет ли воспроизведение
int audio_is_playing(void) {
    return audio_state.is_playing;
}

