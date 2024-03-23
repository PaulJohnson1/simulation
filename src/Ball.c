#include <Ball.h>

#include <stdlib.h>
#include <math.h>

#include <Simulation.h>
#include <Utilities.h>
#include <Renderer/Renderer.h>

void tmp_ball_apply_gravity(struct tmp_ball *b)
{
    // b->acceleration.y += .01;
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
    b->position.x = tmp_clamp(b->position.x, TMP_BALL_RADIUS, TMP_MAP_SIZE - TMP_BALL_RADIUS);
    b->position.y = tmp_clamp(b->position.y, TMP_BALL_RADIUS, TMP_MAP_SIZE - TMP_BALL_RADIUS);
}

void tmp_ball_apply_collision(struct tmp_ball *restrict a, struct tmp_ball *restrict b)
{
    float delta_x = b->position.x - a->position.x;
    float delta_y = b->position.y - a->position.y;
    float d2 = delta_x * delta_x + delta_y * delta_y;
    float collision_radius = TMP_BALL_RADIUS * 2;
    float collision_radius_sq = collision_radius * collision_radius;

    if (d2 < collision_radius_sq && d2 > 0.1f)
    {
        float d = sqrtf(d2);

        float normal_x = delta_x / d + 0.1f;
        float normal_y = delta_y / d + 0.1f;

        float delta = 0.5f * (d - collision_radius);
        float move_x = normal_x * delta;
        float move_y = normal_y * delta;

        a->position.x += move_x;
        a->position.y += move_y;
        b->position.x -= move_x;
        b->position.y -= move_y;
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
    tmp_renderer_circle(r, b->position.x, b->position.y, TMP_BALL_RADIUS);
#endif
}
