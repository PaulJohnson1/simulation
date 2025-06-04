#include "Utilities.h"
#include <Simulation.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <pthread.h>

#include <Ball.h>
#include <Const.h>

void tmp_simulation_init(struct tmp_simulation *s)
{
    memset(s, 0, sizeof *s);
    tmp_collision_manager_init(&s->collisions);
    s->collisions.sim = s;

    for (uint64_t i = 0; i < TMP_BALL_COUNT; i++)
        tmp_simulation_ball_init(s);
}

struct tmp_ball *tmp_simulation_ball_init(struct tmp_simulation *s)
{
    uint32_t id = s->current_id++;
    s->balls[id].position.x = s->balls[id].last_position.x =
        (float)(rand() % (TMP_MAP_SIZE - 2 * TMP_BALL_RADIUS) +
                TMP_BALL_RADIUS);
    s->balls[id].position.y = s->balls[id].last_position.y =
        (float)(rand() % (TMP_MAP_SIZE - 2 * TMP_BALL_RADIUS) +
                TMP_BALL_RADIUS);
    s->balls[id].id = id;
    s->balls[id].simulation = s;
    tmp_collision_manager_insert(&s->collisions, s->balls + id);
    return s->balls + id;
}

static void collide_balls(uint64_t a, uint64_t b, void *c)
{
    struct tmp_simulation *s = (struct tmp_simulation *)c;
    tmp_ball_apply_collision(s->balls + a, s->balls + b);
}

void tmp_simulation_subtick(struct tmp_simulation *s)
{
    struct tmp_ball *balls_end = s->balls + TMP_BALL_COUNT;

    for (struct tmp_ball *i = s->balls; i < balls_end; i++)
        tmp_ball_apply_gravity(i);
    for (int i = 0; i < TMP_BALL_COUNT; i++)
        tmp_collision_manager_entity_from_ball(s->collision_entities + i,
                                               s->balls + i);
    tmp_collision_manager_update_multiple(&s->collisions, s->collision_entities,
                                          s->collision_entities +
                                              TMP_BALL_COUNT);

    tmp_collision_manager_find_possible_collisions(&s->collisions, s,
                                                   collide_balls);

    for (struct tmp_ball *i = s->balls; i < balls_end; i++)
        tmp_ball_apply_constraints(i);
    for (struct tmp_ball *i = s->balls; i < balls_end; i++)
        tmp_ball_tick_verlet(i);
}

void tmp_simulation_tick(struct tmp_simulation *s)
{
    struct timeval start;
    struct timeval end;
    uint64_t elapsed_time;
    static uint64_t average_time = UINT64_MAX;

    gettimeofday(&start, NULL);

    tmp_collision_manager_optimize(&s->collisions);
    for (uint64_t i = 0; i < TMP_SIMULATION_SUBTICKS; i++)
        tmp_simulation_subtick(s);

    gettimeofday(&end, NULL);
    elapsed_time = (uint64_t)((end.tv_sec - start.tv_sec) * 1000000 +
                              (end.tv_usec - start.tv_usec));
    if (average_time == UINT64_MAX)
        average_time = elapsed_time;
    average_time =
        (uint64_t)tmp_lerp((double)average_time, (double)elapsed_time, 0.1);
    double float_average = (double)average_time / 1000.0 / (double)TMP_SIMULATION_SUBTICKS;
    printf("average subtick %.3f mspt\n", float_average);
}

void tmp_simulation_render(struct tmp_simulation *s)
{
#ifdef NRENDER
    return;
#endif

#ifdef TIME_RENDERER
    struct timeval start;
    struct timeval end;

    gettimeofday(&start, NULL);
#endif
    for (struct tmp_ball *i = s->balls; i < s->balls + TMP_BALL_COUNT; i++)
        tmp_ball_render(i);
#ifdef TIME_RENDERER
    gettimeofday(&end, NULL);
    uint64_t elapsed_time = (uint64_t)((end.tv_sec - start.tv_sec) * 1000000 +
                                       (end.tv_usec - start.tv_usec));
    printf("render time: %.3f mspt\n", (float)elapsed_time / 1000.0f);
#endif
}
