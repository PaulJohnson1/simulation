#include <Utilities.h>

#include <assert.h>
#include <math.h>
#include <stdint.h>

float tmp_fclamp(float v, float s, float e)
{
    if (v >= s)
    {
        if (v <= e)
            return v;
        return e;
    }
    return s;
}

float tmp_fast_inverse_root(float x)
{
    int32_t i;
    float x2;
    float y;

    x2 = x * 0.5f;
    y = x;
    i = *(int32_t *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y *= (1.5f - (x2 * y * y));
    // y *= (1.5f - (x2 * y * y));

    return y;
}

#define TRIG_LOOKUP_SIZE 8192

static float sin_table[TRIG_LOOKUP_SIZE];
static float cos_table[TRIG_LOOKUP_SIZE];

__attribute__((constructor)) void tmp_init_lookup_tables()
{
    for (uint64_t i = 0; i < TRIG_LOOKUP_SIZE; i++)
    {
        float a = i;
        sin_table[i] = sinf(a);
        cos_table[i] = cosf(a);
    }
}

float tmp_get_random_sin(uint32_t seed)
{
    return sin_table[seed % TRIG_LOOKUP_SIZE];
}

float tmp_get_random_cos(uint32_t seed)
{
    return cos_table[seed % TRIG_LOOKUP_SIZE];
}
