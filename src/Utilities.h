#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TMP_THREAD_COUNT 8
#define TMP_ROUND_UP(x, y) ((x + (y - 1)) / y)

#ifndef NDEBUG
#include <assert.h>
#define TMP_UNREACHABLE(MESSAGE) assert(!MESSAGE)
#else
#define TMP_UNREACHABLE(MESSAGE) __builtin_unreachable()
#endif

#define tmp_clamp(x, min_val, max_val)                                         \
    ((x) < (min_val) ? (min_val) : ((x) > (max_val) ? (max_val) : (x)))
#define tmp_ceil(x, y) (((x) + (y)-1) / (y))
#define tmp_lerp(s, e, t) ((1.0 - (t)) * (s) + (t) * (e))

void tmp_log_hex(uint8_t *, uint8_t *);
float tmp_angle_lerp(float, float, float);
int tmp_angle_within(float, float, float);
float tmp_frand();
float tmp_fclamp(float, float, float);
char *tmp_sprintf(char *, double);
float tmp_fast_inverse_root(float);
void tmp_init_lookup_tables();
float tmp_get_random_sin(uint32_t seed);
float tmp_get_random_cos(uint32_t seed);

#define TMP_SET_IF_GREATER(var, num)                                           \
    {                                                                          \
        if ((var) < (num))                                                     \
            (var) = (num);                                                     \
    }
#define TMP_SET_IF_LESS(var, num)                                              \
    {                                                                          \
        if ((num) < (var))                                                     \
            (var) = (num);                                                     \
    }

#define tmp_vector_declare(type, name)                                         \
    type *name;                                                                \
    type *name##_end;                                                          \
    type *name##_cap;

#define tmp_vector_declare_zero(type, name)                                    \
    type *name = 0;                                                            \
    type *name##_end = 0;                                                      \
    type *name##_cap = 0;

#define tmp_vector_grow(type, name)                                            \
    if (name##_end >= name##_cap)                                              \
    {                                                                          \
        uint64_t capacity = name##_cap - name;                                 \
        type *new_data =                                                       \
            (type *)realloc(name, (capacity * 2 + 1) * sizeof *name);          \
        if (!new_data)                                                         \
            perror("no mem");                                                  \
        type *new_data_cap = new_data + capacity * 2 + 1;                      \
        name = new_data;                                                       \
        name##_end = new_data + capacity;                                      \
        name##_cap = new_data_cap;                                             \
    }

#define tmp_vector_size(name) (name##_end - name)

#ifdef __cplusplus
#define restrict
#endif
