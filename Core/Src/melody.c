#include "melody.h"
#include "audio_interface.h"
#include <stdlib.h>
#include <string.h>

// Структура для состояния воспроизведения мелодии
typedef struct {
    Melody* melody;
    size_t current_note_index;
    int is_in_pause;  // Флаг: 1 если сейчас пауза, 0 если нота
    void (*completion_callback)(void*);
    void* completion_user_data;
} melody_playback_state_t;

// Callback для перехода к следующей ноте (вызывается после завершения ноты или паузы)
static void melody_note_callback(void* user_data) {
    melody_playback_state_t* state = (melody_playback_state_t*)user_data;
    
    if (!state || !state->melody || !state->melody->notes) {
        return;
    }
    
    // Если мы только что завершили паузу, переходим к следующей ноте
    if (state->is_in_pause) {
        state->is_in_pause = 0;
        state->current_note_index++;
        
        // Если есть еще ноты, воспроизводим следующую
        if (state->current_note_index < state->melody->count) {
            Note* next_note = &state->melody->notes[state->current_note_index];
            audio_start_note(next_note->frequency, next_note->duration, melody_note_callback, state);
        } else {
            // Мелодия завершена
            if (state->completion_callback) {
                state->completion_callback(state->completion_user_data);
            }
            // Освобождаем состояние
            free(state);
        }
        return;
    }
    
    // Мы только что завершили ноту, проверяем паузу
    if (state->current_note_index < state->melody->count) {
        Note* current_note = &state->melody->notes[state->current_note_index];
        
        // Если есть пауза после этой ноты, запускаем её
        if (current_note->pause > 0) {
            state->is_in_pause = 1;
            audio_start_pause(current_note->pause, melody_note_callback, state);
            return;
        }
    }
    
    // Паузы нет, переходим к следующей ноте
    state->current_note_index++;
    
    // Если есть еще ноты, воспроизводим следующую
    if (state->current_note_index < state->melody->count) {
        Note* next_note = &state->melody->notes[state->current_note_index];
        audio_start_note(next_note->frequency, next_note->duration, melody_note_callback, state);
    } else {
        // Мелодия завершена
        if (state->completion_callback) {
            state->completion_callback(state->completion_user_data);
        }
        // Освобождаем состояние
        free(state);
    }
}

// Создание новой мелодии
Melody* melody_create(size_t count, const char* name) {
    Melody* melody = (Melody*)malloc(sizeof(Melody));
    if (!melody) {
        return NULL;
    }
    
    melody->notes = (Note*)malloc(sizeof(Note) * count);
    if (!melody->notes) {
        free(melody);
        return NULL;
    }
    
    melody->count = count;
    
    // Копируем название, если оно задано
    if (name) {
        melody->name = (char*)malloc(strlen(name) + 1);
        if (melody->name) {
            strcpy(melody->name, name);
        } else {
            melody->name = NULL;
        }
    } else {
        melody->name = NULL;
    }
    
    return melody;
}

// Освобождение памяти мелодии
void melody_free(Melody* melody) {
    if (melody) {
        if (melody->name) {
            free(melody->name);
        }
        if (melody->notes) {
            free(melody->notes);
        }
        free(melody);
    }
}

// Воспроизведение мелодии (асинхронное)
// completion_callback вызывается когда мелодия завершена (может быть NULL)
// completion_user_data передается в completion_callback (может быть NULL)
void play_melody(Melody* melody, void (*completion_callback)(void*), void* completion_user_data) {
    if (!melody || !melody->notes || melody->count == 0) {
        if (completion_callback) {
            completion_callback(completion_user_data);
        }
        return;
    }
    
    // Создаем состояние воспроизведения
    melody_playback_state_t* state = (melody_playback_state_t*)malloc(sizeof(melody_playback_state_t));
    
    state->melody = melody;
    state->current_note_index = 0;
    state->is_in_pause = 0;
    state->completion_callback = completion_callback;
    state->completion_user_data = completion_user_data;
    
    // Запускаем первую ноту
    Note* first_note = &melody->notes[0];
    audio_start_note(first_note->frequency, first_note->duration, melody_note_callback, state);
}

