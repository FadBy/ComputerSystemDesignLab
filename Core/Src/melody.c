#include "melody.h"
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
