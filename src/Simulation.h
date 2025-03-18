#pragma once

#include <SpatialHash.h>

struct tmp_ball;

struct tmp_simulation
{
    struct tmp_spatial_hash grid;
    struct tmp_ball *balls;
    struct tmp_ball *balls_end;
    struct tmp_ball *balls_cap;
    uint64_t current_id;
};

extern void tmp_simulation_init(struct tmp_simulation *);
struct tmp_ball *tmp_simulation_ball_init(struct tmp_simulation *);
extern void tmp_simulation_tick(struct tmp_simulation *, float delta);
extern void tmp_simulation_render(struct tmp_simulation *);
