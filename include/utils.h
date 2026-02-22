#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/* Минимальное и максимальное */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* Ограничение значения в диапазоне (безопасная версия) */
#define CLAMP(x, min, max) \
    ((x) < (min) ? (min) : \
     ((x) > (max) ? (max) : (x)))

/* Абсолютное значение для целых */
static inline int16_t abs_i16(int16_t x) {
    return (x < 0) ? -x : x;
}

/* Картирование значения из одного диапазона в другой */
static inline uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, 
                           uint32_t out_min, uint32_t out_max) {
    if (x < in_min) x = in_min;
    if (x > in_max) x = in_max;
    if (in_max == in_min) return out_min; /* Защита от деления на 0 */
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/* Задержка в циклах (приблизительно) */
static inline void delay_cycles(uint32_t cycles) {
    for(volatile uint32_t i = 0; i < cycles; i++) {
        __asm__("nop");
    }
}

/* Безопасное ограничение для uint8_t */
static inline uint8_t clamp_u8(uint8_t x, uint8_t min, uint8_t max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

#endif /* UTILS_H */