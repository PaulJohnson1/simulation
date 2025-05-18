#pragma once

#include <Collision/CollisionManager.h>
#include <Const.h>
#include <Utilities.h>

struct tmp_ball;

struct tmp_simulation
{
    struct tmp_collision_manager collisions;
    struct tmp_collision_manager_entity collision_entities[TMP_BALL_COUNT];
    tmp_vector_declare(struct tmp_ball, balls);
    uint32_t current_id;
};

extern void tmp_simulation_init(struct tmp_simulation *);
extern struct tmp_ball *tmp_simulation_ball_init(struct tmp_simulation *);
extern void tmp_simulation_subtick(struct tmp_simulation *, float delta);
extern void tmp_simulation_tick(struct tmp_simulation *, float delta);
extern void tmp_simulation_render(struct tmp_simulation *);
