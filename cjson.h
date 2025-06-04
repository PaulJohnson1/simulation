#pragma once

#include <stdint.h>
#include <stdlib.h>

#define cjson_vector_declare(type, name)                                       \
    type *name;                                                                \
    type *name##_end;                                                          \
    type *name##_cap;

#define cjson_vector_declare_zero(type, name)                                  \
    type *name = 0;                                                            \
    type *name##_end = 0;                                                      \
    type *name##_cap = 0;

#define cjson_vector_grow(type, name)                                          \
    do                                                                         \
    {                                                                          \
        if (name##_end >= name##_cap)                                          \
        {                                                                      \
            uint64_t capacity = (uint64_t)(name##_cap - name);                 \
            type *new_data =                                                   \
                (type *)realloc(name, (capacity * 2 + 1) * sizeof *name);      \
            type *new_data_cap = new_data + capacity * 2 + 1;                  \
            name = new_data;                                                   \
            name##_end = new_data + capacity;                                  \
            name##_cap = new_data_cap;                                         \
        }                                                                      \
    } while (0)

#define cjson_vector_reserve(type, n, name)                                    \
    do                                                                         \
    {                                                                          \
        long old_size = cjson_vector_size(name);                               \
        while (name##_end + (n) >= name##_cap)                                 \
        {                                                                      \
            cjson_vector_grow(type, name);                                     \
            name##_end++;                                                      \
        }                                                                      \
        name##_end = name + old_size;                                          \
    } while (0)

#define cjson_vector_size(name) (name##_end - name)

struct cjson_object_entry
{
    char const *name;
    struct cjson_value *value;
};

struct cjson_object
{
    cjson_vector_declare(struct cjson_object_entry, values);
};

struct cjson_string
{
    char const *value;
};

struct cjson_number
{
    double value;
};

struct cjson_bool
{
    int value : 1;
};

struct cjson_null
{
};

struct cjson_array
{
    cjson_vector_declare(struct cjson_value *, values);
};

enum cjson_value_type
{
    cjson_value_type_object,
    cjson_value_type_string,
    cjson_value_type_number,
    cjson_value_type_array,
    cjson_value_type_bool,
    cjson_value_type_null,

    cjson_value_type_max
};

// tagged union
struct cjson_value
{
    enum cjson_value_type type;
    union
    {
        struct cjson_object object;
        struct cjson_string string;
        struct cjson_number number;
        struct cjson_array array;
        struct cjson_bool bool_;
        struct cjson_null null;
    };
};

char const *cjson_value_stringify(struct cjson_value *);
void cjson_value_free(struct cjson_value const *);
