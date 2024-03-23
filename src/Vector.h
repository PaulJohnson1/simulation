#pragma once

struct tmp_vector
{
    float x;
    float y;
};

extern void tmp_vector_set(struct tmp_vector *, float, float);
extern void tmp_vector_add(struct tmp_vector *, struct tmp_vector *);
extern void tmp_vector_sub(struct tmp_vector *, struct tmp_vector *);
extern void tmp_vector_scale(struct tmp_vector *, float);
extern int tmp_vector_equals(struct tmp_vector *, struct tmp_vector *);
extern void tmp_vector_normalize(struct tmp_vector *);
extern void tmp_vector_set_magnitude(struct tmp_vector *, float);
extern float tmp_vector_get_magnitude(struct tmp_vector *);
extern float tmp_vector_theta(struct tmp_vector *);
extern void tmp_vector_from_polar(struct tmp_vector *, float r, float theta);
