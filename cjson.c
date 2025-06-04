#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "cjson.h"

char const *cjson_value_stringify(struct cjson_value *value)
{
    switch (value->type)
    {
    case cjson_value_type_null:
        return strdup("null");
    case cjson_value_type_bool:
        return strdup(value->bool_.value ? "true" : "false");
    case cjson_value_type_number:
    {
        char buffer[100];
        buffer[sprintf(buffer, "%g", value->number.value)] = 0;
        return strdup(buffer);
    }
    case cjson_value_type_string:
    {
        // Calculate required buffer size for escaped string
        size_t escaped_len = 2; // Start with two quotes
        for (const char *p = value->string.value; *p; ++p)
        {
            switch (*p)
            {
            case '"':
                escaped_len += 2;
                break;
            case '\\':
                escaped_len += 2;
                break;
            case '\b':
                escaped_len += 2;
                break;
            case '\f':
                escaped_len += 2;
                break;
            case '\n':
                escaped_len += 2;
                break;
            case '\r':
                escaped_len += 2;
                break;
            case '\t':
                escaped_len += 2;
                break;
            default:
                if ((unsigned char)*p < 0x20)
                    escaped_len += 6;
                else
                    escaped_len += 1;
            }
        }

        // Allocate and build escaped string
        char *result = malloc(escaped_len + 1);
        char *dest = result;
        *dest++ = '"';
        for (const char *p = value->string.value; *p; ++p)
        {
            switch (*p)
            {
            case '"':
                memcpy(dest, "\\\"", 2);
                dest += 2;
                break;
            case '\\':
                memcpy(dest, "\\\\", 2);
                dest += 2;
                break;
            case '\b':
                memcpy(dest, "\\b", 2);
                dest += 2;
                break;
            case '\f':
                memcpy(dest, "\\f", 2);
                dest += 2;
                break;
            case '\n':
                memcpy(dest, "\\n", 2);
                dest += 2;
                break;
            case '\r':
                memcpy(dest, "\\r", 2);
                dest += 2;
                break;
            case '\t':
                memcpy(dest, "\\t", 2);
                dest += 2;
                break;
            default:
                if ((unsigned char)*p < 0x20)
                {
                    sprintf(dest, "\\u%04x", (unsigned char)*p);
                    dest += 6;
                }
                else
                {
                    *dest++ = *p;
                }
            }
        }
        *dest++ = '"';
        *dest = 0;
        return result;
    }
    case cjson_value_type_array:
    {
        cjson_vector_declare_zero(char, str);
        cjson_vector_grow(char, str);
        *str_end++ = '[';
        for (size_t i = 0; i < cjson_vector_size(value->array.values); i++)
        {
            if (i > 0)
            {
                cjson_vector_grow(char, str);
                *str_end++ = ',';
            }
            char *element =
                (char *)cjson_value_stringify(value->array.values[i]);
            size_t len = strlen(element);
            cjson_vector_reserve(char, len, str);
            memcpy(str_end, element, len);
            str_end += len;
            free(element);
        }
        cjson_vector_grow(char, str);
        *str_end++ = ']';
        *str_end = 0;
        return str;
    }
    case cjson_value_type_object:
    {
        cjson_vector_declare_zero(char, str);
        cjson_vector_grow(char, str);
        *str_end++ = '{';
        for (size_t i = 0; i < cjson_vector_size(value->object.values); i++)
        {
            if (i > 0)
            {
                cjson_vector_grow(char, str);
                *str_end++ = ',';
            }

            // Stringify key
            struct cjson_object_entry *entry = &value->object.values[i];
            size_t key_len = strlen(entry->name);
            cjson_vector_grow(char, str);
            *str_end++ = '"';
            for (const char *p = entry->name; *p; p++)
            {
                if (*p == '"' || *p == '\\')
                {
                    cjson_vector_grow(char, str);
                    *str_end++ = '\\';
                }
                cjson_vector_grow(char, str);
                *str_end++ = *p;
            }
            cjson_vector_grow(char, str);
            *str_end++ = '"';
            cjson_vector_grow(char, str);
            *str_end++ = ':';

            // Stringify value
            char *val_str = (char *)cjson_value_stringify(entry->value);
            size_t val_len = strlen(val_str);
            cjson_vector_reserve(char, val_len, str);
            memcpy(str_end, val_str, val_len);
            str_end += val_len;
            free(val_str);
        }
        cjson_vector_grow(char, str);
        *str_end++ = '}';
        cjson_vector_grow(char, str);
        *str_end = 0;
        return str;
    }
    case cjson_value_type_max:
        assert(0);
    }

    assert(0);
    return NULL;
}

void cjson_value_free(struct cjson_value const *value)
{
    switch (value->type)
    {
    case cjson_value_type_string:
        free((void *)value->string.value);
        break;
    case cjson_value_type_array:
        free(value->array.values);
        break;
    case cjson_value_type_object:
        for (struct cjson_object_entry *i = value->object.values;
             i < value->object.values_end; i++)
        {
            free((void *)i->name);
            cjson_value_free(i->value);
        }
        free(value->object.values);
        break;
    default:
        break;
    }
}
