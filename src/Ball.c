#include <Ball.h>

#include <math.h>
#include <stdlib.h>

#include <Renderer/Renderer.h>
#include <Simulation.h>
#include <Utilities.h>

void tmp_ball_apply_gravity(struct tmp_ball *b)
{
    static uint64_t r = 1;
    r ^= r >> 12;
    r ^= r << 25;
    r ^= r >> 27;
    float acc_x = 0.25f * tmp_get_random_cos(r);
    float acc_y = 0.25f * tmp_get_random_sin(r);
    b->acceleration.x += acc_x;
    b->acceleration.y += acc_y;
}

void tmp_ball_apply_constraints(struct tmp_ball *b)
{
    b->position.x = tmp_clamp(b->position.x, TMP_BALL_RADIUS,
                              TMP_MAP_SIZE - TMP_BALL_RADIUS);
    b->position.y = tmp_clamp(b->position.y, TMP_BALL_RADIUS,
                              TMP_MAP_SIZE - TMP_BALL_RADIUS);
}

void tmp_ball_apply_collision(struct tmp_ball *restrict a,
                              struct tmp_ball *restrict b)
{
    const float dx = b->position.x - a->position.x;
    const float dy = b->position.y - a->position.y;
    const float d2 = dx * dx + dy * dy;
    const float min_distance = TMP_BALL_RADIUS * 2.0f;
    const float min_distance_sq = min_distance * min_distance;

    if (d2 < min_distance_sq && d2 > 0.1f)
    {
        const float inv_d = tmp_fast_inverse_root(d2);
        const float normal_scale = -0.01f + (1.0f / inv_d - min_distance) * 0.5f;
        const float nx = dx * inv_d;
        const float ny = dy * inv_d;

        a->position.x += nx * normal_scale;
        a->position.y += ny * normal_scale;
        b->position.x -= nx * normal_scale;
        b->position.y -= ny * normal_scale;
    }
}

struct tmp_vector tmp_ball_get_velocity(struct tmp_ball *b)
{
    struct tmp_vector v = b->position;
    tmp_vector_sub(&v, &b->last_position);
    return v;
}

void tmp_ball_tick_verlet(struct tmp_ball *b, float dt)
{
    struct tmp_vector displacement = tmp_ball_get_velocity(b);

    b->last_position = b->position;

    b->position.x += displacement.x + b->acceleration.x * dt * dt;
    b->position.y += displacement.y + b->acceleration.y * dt * dt;

    b->acceleration.x = 0;
    b->acceleration.y = 0;
}

void tmp_ball_render(struct tmp_ball *b, struct tmp_renderer *r)
{
#ifdef WASM_BUILD
    tmp_renderer_begin_path(r);
    tmp_renderer_arc(r, b->position.x, b->position.y, TMP_BALL_RADIUS);
    tmp_renderer_fill(r);
#else
    if (b->id % 5 == 0)
        tmp_renderer_circle(r, b->position.x, b->position.y, TMP_BALL_RADIUS);
#endif
}
