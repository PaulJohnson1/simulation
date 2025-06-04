#pragma once

#include <Ball.h>
#include <Collision/CollisionManager.h>
#include <Const.h>
#include <Utilities.h>

struct tmp_simulation
{
    struct tmp_collision_manager collisions;
    struct tmp_collision_manager_entity collision_entities[TMP_BALL_COUNT];
    struct tmp_ball balls[TMP_BALL_COUNT];
    uint32_t current_id;
};

CPLUSPLUS_PREFIX
extern void tmp_simulation_init(struct tmp_simulation *);
extern struct tmp_ball *tmp_simulation_ball_init(struct tmp_simulation *);
extern void tmp_simulation_subtick(struct tmp_simulation *);
extern void tmp_simulation_tick(struct tmp_simulation *);
extern void tmp_simulation_render(struct tmp_simulation *);
CPLUSPLUS_SUFFIX
