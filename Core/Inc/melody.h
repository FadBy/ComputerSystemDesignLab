#ifndef MELODY_H
#define MELODY_H

#include <stdint.h>
#include <stddef.h>

// Структура для одной ноты
typedef struct {
    uint32_t frequency;  // Частота в Гц
    uint32_t duration;   // Длительность в мс
    uint32_t pause;      // Пауза после ноты в мс
} Note;

// Структура для мелодии
typedef struct {
    char* name;          // Название мелодии (может быть NULL)
    Note* notes;         // Массив нот
    size_t count;        // Количество нот
} Melody;

// Функции для работы с мелодиями
// name - название мелодии (может быть NULL, будет скопировано)
Melody* melody_create(size_t count, const char* name);
void melody_free(Melody* melody);

#endif // MELODY_H

