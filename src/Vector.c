#include <Vector.h>

#include <float.h>
#include <math.h>

#include <Utilities.h>

void tmp_vector_set(struct tmp_vector *vector, float x, float y)
{
    vector->x = x;
    vector->y = y;
}

void tmp_vector_add(struct tmp_vector *vector, struct tmp_vector *right)
{
    vector->x += right->x;
    vector->y += right->y;
}

void tmp_vector_sub(struct tmp_vector *vector, struct tmp_vector *right)
{
    vector->x -= right->x;
    vector->y -= right->y;
}

void tmp_vector_scale(struct tmp_vector *vector, float v)
{
    vector->x *= v;
    vector->y *= v;
}

int tmp_vector_equals(struct tmp_vector *vector, struct tmp_vector *other)
{
    return vector->x == other->x && vector->y == other->y;
}

void tmp_vector_normalize(struct tmp_vector *vector)
{
    float d2 = vector->x * vector->x + vector->y * vector->y;
    if (d2 <= FLT_EPSILON)
    {
        vector->x = 1;
        vector->y = 0;
        return;
    }
    float reciprocal_d = tmp_fast_inverse_root(d2);
    tmp_vector_scale(vector, reciprocal_d);
}

void tmp_vector_set_magnitude(struct tmp_vector *vector, float mag)
{
    if (vector->x == 0 && vector->y == 0)
        return;
    tmp_vector_scale(
        vector, mag / sqrtf(vector->x * vector->x + vector->y * vector->y));
}

float tmp_vector_get_magnitude(struct tmp_vector *vector)
{
    return sqrtf(vector->x * vector->x + vector->y * vector->y);
}

float tmp_vector_theta(struct tmp_vector *vector)
{
    return atan2f(vector->y, vector->x);
}

void tmp_vector_from_polar(struct tmp_vector *vector, float radius, float angle)
{
    vector->x = radius * cosf(angle);
    vector->y = radius * sinf(angle);
}
