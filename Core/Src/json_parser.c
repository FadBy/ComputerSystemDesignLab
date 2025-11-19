#include "json_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Извлечение имени файла без пути и расширения
static char* extract_filename(const char* filepath) {
    if (!filepath) {
        return NULL;
    }
    
    // Находим последний '/' или '\'
    const char* filename_start = filepath;
    const char* p = filepath;
    while (*p) {
        if (*p == '/' || *p == '\\') {
            filename_start = p + 1;
        }
        p++;
    }
    
    // Находим точку расширения
    const char* ext_start = strrchr(filename_start, '.');
    size_t name_len;
    if (ext_start) {
        name_len = ext_start - filename_start;
    } else {
        name_len = strlen(filename_start);
    }
    
    if (name_len == 0) {
        return NULL;
    }
    
    // Выделяем память и копируем имя
    char* name = (char*)malloc(name_len + 1);
    if (!name) {
        return NULL;
    }
    
    strncpy(name, filename_start, name_len);
    name[name_len] = '\0';
    
    return name;
}

// Пропуск пробельных символов
static void skip_whitespace(const char** str) {
    while (**str && isspace(**str)) {
        (*str)++;
    }
}

// Пропуск символа, если он совпадает
static int skip_char(const char** str, char c) {
    skip_whitespace(str);
    if (**str == c) {
        (*str)++;
        return 1;
    }
    return 0;
}

// Парсинг числа
static int parse_number(const char** str, uint32_t* value) {
    skip_whitespace(str);
    
    if (!**str || !isdigit(**str)) {
        return 0;
    }
    
    *value = 0;
    while (**str && isdigit(**str)) {
        *value = *value * 10 + (**str - '0');
        (*str)++;
    }
    
    return 1;
}

// Парсинг строки (ключа JSON)
static int parse_string(const char** str, char* buffer, size_t buffer_size) {
    skip_whitespace(str);
    
    if (**str != '"') {
        return 0;
    }
    (*str)++;
    
    size_t i = 0;
    while (**str && **str != '"' && i < buffer_size - 1) {
        buffer[i++] = **str;
        (*str)++;
    }
    
    if (**str != '"') {
        return 0;
    }
    (*str)++;
    
    buffer[i] = '\0';
    return 1;
}

// Парсинг одного объекта ноты
static int parse_note_object(const char** str, Note* note) {
    skip_whitespace(str);
    
    if (!skip_char(str, '{')) {
        return 0;
    }
    
    uint32_t frequency = 0, duration = 0, pause = 0;
    int has_frequency = 0, has_duration = 0, has_pause = 0;
    
    while (**str && **str != '}') {
        char key[32];
        
        if (!parse_string(str, key, sizeof(key))) {
            return 0;
        }
        
        if (!skip_char(str, ':')) {
            return 0;
        }
        
        uint32_t value;
        if (!parse_number(str, &value)) {
            return 0;
        }
        
        if (strcmp(key, "frequency") == 0) {
            frequency = value;
            has_frequency = 1;
        } else if (strcmp(key, "duration") == 0) {
            duration = value;
            has_duration = 1;
        } else if (strcmp(key, "pause") == 0) {
            pause = value;
            has_pause = 1;
        }
        
        // Пропуск запятой, если есть
        skip_char(str, ',');
    }
    
    if (!skip_char(str, '}')) {
        return 0;
    }
    
    if (!has_frequency || !has_duration) {
        return 0;
    }
    
    note->frequency = frequency;
    note->duration = duration;
    note->pause = has_pause ? pause : 0;
    
    return 1;
}

// Парсинг мелодии из JSON файла
Melody* parse_melody_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }
    
    // Определяем размер файла
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(file);
        return NULL;
    }
    
    // Читаем весь файл
    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    size_t read_size = fread(buffer, 1, file_size, file);
    fclose(file);
    buffer[read_size] = '\0';
    
    const char* str = buffer;
    skip_whitespace(&str);
    
    // Пропускаем открывающую скобку массива
    if (!skip_char(&str, '[')) {
        free(buffer);
        return NULL;
    }
    
    // Подсчитываем количество нот (простой способ - считаем '{')
    size_t note_count = 0;
    const char* temp = str;
    while (*temp) {
        if (*temp == '{') {
            note_count++;
        }
        temp++;
    }
    
    if (note_count == 0) {
        free(buffer);
        return NULL;
    }
    
    // Извлекаем имя файла
    char* melody_name = extract_filename(filename);
    
    // Создаем мелодию
    Melody* melody = melody_create(note_count, melody_name);
    if (!melody) {
        free(melody_name);
        free(buffer);
        return NULL;
    }
    
    // Освобождаем временное имя (оно уже скопировано в мелодию)
    free(melody_name);
    
    // Парсим ноты
    size_t parsed_count = 0;
    while (*str && *str != ']' && parsed_count < note_count) {
        if (!parse_note_object(&str, &melody->notes[parsed_count])) {
            melody_free(melody);
            free(buffer);
            return NULL;
        }
        
        parsed_count++;
        
        // Пропуск запятой между объектами
        skip_char(&str, ',');
    }
    
    melody->count = parsed_count;
    
    free(buffer);
    return melody;
}

