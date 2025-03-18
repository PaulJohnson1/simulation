#include <Simulation.h>

#include <math.h>
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
    // skip id 0 (it's the null entity)
    s->current_id = 1;
    tmp_vector_grow(struct tmp_ball, s->balls);
    s->balls_end++;
    // s->grid_entities = malloc(sizeof *s->grid_entities * TMP_BALL_COUNT);
    tmp_spatial_hash_init(&s->grid);

    // struct tmp_ball *a = tmp_simulation_ball_init(s);
    // struct tmp_ball *b = tmp_simulation_ball_init(s);
    // a->position.x = a->last_position.x = -200 + TMP_MAP_SIZE / 2;
    // a->position.y = a->last_position.y = -0 + TMP_MAP_SIZE / 2;
    // b->position.x = b->last_position.x = 200 + TMP_MAP_SIZE / 2;
    // b->position.y = b->last_position.y = 0 + TMP_MAP_SIZE / 2;

    // a->acceleration.x += 0;
    // b->acceleration.x -= 20;

    // tmp_spatial_hash_insert(&s->grid, a);
    // tmp_spatial_hash_insert(&s->grid, b);

    for (uint64_t i = 1; i < TMP_BALL_COUNT; i++)
        tmp_spatial_hash_insert(&s->grid, tmp_simulation_ball_init(s));
}

struct tmp_ball *tmp_simulation_ball_init(struct tmp_simulation *s)
{
    tmp_vector_grow(struct tmp_ball, s->balls);
    s->balls_end->position.x = s->balls_end->last_position.x =
        rand() % (TMP_MAP_SIZE - 2 * TMP_BALL_RADIUS) + TMP_BALL_RADIUS;
    s->balls_end->position.y = s->balls_end->last_position.y =
        rand() % (TMP_MAP_SIZE - 2 * TMP_BALL_RADIUS) + TMP_BALL_RADIUS;
    s->balls_end->id = s->current_id++;
    s->balls_end->simulation = s;
    return s->balls_end++;
}

static void collide_balls(uint64_t a, uint64_t b, void *c)
{
    struct tmp_simulation *s = c;
    tmp_ball_apply_collision(s->balls + a, s->balls + b);
}

void tmp_simulation_tick(struct tmp_simulation *s, float dt)
{
    struct timeval start;
    struct timeval end;
    uint64_t elapsed_time;
    uint64_t steps = 10;
    float sim_dt = 1 / (float)steps;
    static uint64_t average_time = -1;

    tmp_spatial_hash_optimize(&s->grid);

    for (uint64_t i = 0; i < steps; i++)
    {
        gettimeofday(&start, NULL);
        for (struct tmp_ball *i = s->balls + 1; i < s->balls_end; i++)
        {
            tmp_ball_apply_gravity(i);
        }
        tmp_spatial_hash_construct_entities(&s->grid, TMP_BALL_COUNT, s->balls);
        tmp_spatial_hash_update_multiple(&s->grid);
        tmp_spatial_hash_find_possible_collisions(&s->grid, s, collide_balls);

        for (struct tmp_ball *i = s->balls + 1; i < s->balls_end; i++)
        {
            tmp_ball_apply_constraints(i);
            tmp_ball_tick_verlet(i, sim_dt);
        }

        gettimeofday(&end, NULL);
        elapsed_time = ((end.tv_sec - start.tv_sec) * 1000000 +
                        (end.tv_usec - start.tv_usec));
        if (average_time == -1)
            average_time = elapsed_time;
        average_time = tmp_lerp(average_time, elapsed_time, 0.01f);
    }
    printf("average subtick %lu.%lu mspt\n", average_time / 1000,
           average_time % 1000);
}

void tmp_simulation_render(struct tmp_simulation *s)
{
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    for (struct tmp_ball *i = s->balls + 1; i < s->balls_end; i++)
        tmp_ball_render(i);
    gettimeofday(&end, NULL);
    uint64_t elapsed_time =
        ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec));
    printf("render %lu.%lu ms\n", elapsed_time / 1000, elapsed_time % 1000);
}
