#pragma once

#include <stdint.h>

#include <Vector.h>

struct tmp_renderer;

struct tmp_ball
{
    struct tmp_simulation *simulation;
    struct tmp_vector position;
    struct tmp_vector last_position;
    struct tmp_vector acceleration;
    uint64_t current_shg_cell;
    uint32_t id;
};

extern void tmp_ball_apply_constraints(struct tmp_ball *);
extern void tmp_ball_apply_gravity(struct tmp_ball *);
extern void tmp_ball_apply_collision(struct tmp_ball *a, struct tmp_ball *b);
extern void tmp_ball_tick_verlet(struct tmp_ball *, float delta);
extern void tmp_ball_render(struct tmp_ball *, struct tmp_renderer *);
extern struct tmp_vector tmp_ball_get_velocity(struct tmp_ball *);