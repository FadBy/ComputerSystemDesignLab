#include "melodies_data.h"
#include "melody.h"
#include <stdlib.h>
#include <string.h>

// Happy Birthday
static const Note happy_birthday_notes[] = {
    {262, 100, 50}, {262, 100, 50}, {294, 400, 50}, {262, 400, 50}, {349, 400, 50}, {330, 800, 100},
    {262, 200, 50}, {262, 100, 50}, {294, 400, 50}, {262, 400, 50}, {392, 400, 50}, {349, 800, 100},
    {262, 200, 50}, {262, 100, 50}, {523, 400, 50}, {440, 400, 50}, {349, 400, 50}, {330, 400, 50}, {294, 800, 100},
    {466, 200, 50}, {466, 100, 50}, {440, 400, 50}, {349, 400, 50}, {392, 400, 50}, {349, 800, 100}
};

Melody* get_happy_birthday(void) {
    Melody* m = melody_create(sizeof(happy_birthday_notes) / sizeof(Note), "Happy Birthday");
    if (!m) return NULL;
    memcpy(m->notes, happy_birthday_notes, sizeof(happy_birthday_notes));
    return m;
}

// Jingle Bells
static const Note jingle_bells_notes[] = {
    {523, 250, 50}, {523, 250, 50}, {523, 500, 50},
    {523, 250, 50}, {523, 250, 50}, {523, 500, 50},
    {523, 250, 50}, {659, 250, 50}, {440, 250, 50}, {494, 250, 50}, {523, 1000, 100},
    {587, 250, 50}, {587, 250, 50}, {587, 250, 50}, {587, 250, 50},
    {587, 250, 50}, {523, 250, 50}, {523, 250, 50}, {523, 250, 50},
    {523, 250, 50}, {494, 250, 50}, {494, 250, 50}, {523, 250, 50}, {494, 500, 50}, {659, 500, 100}
};

Melody* get_jingle_bells(void) {
    Melody* m = melody_create(sizeof(jingle_bells_notes) / sizeof(Note), "Jingle Bells");
    if (!m) return NULL;
    memcpy(m->notes, jingle_bells_notes, sizeof(jingle_bells_notes));
    return m;
}

// В лесу родилась ёлочка
static const Note elochka_notes[] = {
    {330, 300, 50}, {349, 300, 50}, {392, 600, 50},
    {392, 300, 50}, {440, 300, 50}, {494, 600, 50},
    {523, 200, 50}, {494, 200, 50}, {440, 200, 50}, {392, 400, 50},
    {330, 300, 50}, {349, 300, 50}, {392, 600, 50},
    {392, 300, 50}, {440, 300, 50}, {494, 600, 50},
    {523, 200, 50}, {494, 200, 50}, {440, 200, 50}, {392, 600, 100},
    {262, 300, 50}, {294, 300, 50}, {330, 300, 50}, {349, 300, 50},
    {392, 600, 50}, {440, 300, 50}, {494, 600, 50},
    {523, 200, 50}, {494, 200, 50}, {440, 200, 50}, {392, 600, 100}
};

Melody* get_elochka(void) {
    Melody* m = melody_create(sizeof(elochka_notes) / sizeof(Note), "V lesu rodilas elochka");
    if (!m) return NULL;
    memcpy(m->notes, elochka_notes, sizeof(elochka_notes));
    return m;
}

// Imperial March (Theme of Darth Vader)
static const Note imperial_march_notes[] = {
    {440, 500, 50}, {440, 500, 50}, {440, 500, 50},
    {349, 350, 50}, {523, 150, 50},
    {440, 500, 50}, {349, 350, 50}, {523, 150, 50}, {440, 1000, 100},
    {659, 500, 50}, {659, 500, 50}, {659, 500, 50},
    {698, 350, 50}, {523, 150, 50},
    {415, 500, 50}, {349, 350, 50}, {523, 150, 50}, {440, 1000, 100},
    {880, 500, 50}, {440, 350, 50}, {440, 150, 50},
    {880, 500, 50}, {831, 250, 50}, {784, 250, 50},
    {740, 125, 50}, {698, 125, 50}, {740, 250, 50},
    {466, 250, 50}, {523, 350, 50}, {349, 150, 50}, {415, 250, 50}, {349, 150, 50}, {523, 1000, 100}
};

Melody* get_imperial_march(void) {
    Melody* m = melody_create(sizeof(imperial_march_notes) / sizeof(Note), "Imperial March");
    if (!m) return NULL;
    memcpy(m->notes, imperial_march_notes, sizeof(imperial_march_notes));
    return m;
}

