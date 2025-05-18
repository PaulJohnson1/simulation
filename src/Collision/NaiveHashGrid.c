#ifdef TMP_USE_NAIVE_HASH_GRID
#include <Collision/NaiveHashGrid.h>

#include <stdlib.h>
#include <string.h>

#include <Ball.h>
#include <Utilities.h>

#define HASH_FUNCTION_FACTOR TMP_NAIVE_HASH_GRID_CELL_COUNT_AXIS
#define HASH_FUNCTION(X, Y) ((X) + HASH_FUNCTION_FACTOR * (Y))

void tmp_naive_hash_grid_init(struct tmp_naive_hash_grid *m)
{
    memset(m, 0, sizeof *m);
}

void tmp_naive_hash_grid_insert(struct tmp_naive_hash_grid *m,
                                struct tmp_ball const *b)
{
    uint16_t x_min = tmp_clamp((uint16_t)(b->position.x - TMP_BALL_RADIUS), 0,
                               TMP_MAP_SIZE - 1) /
                     TMP_SPATIAL_HASH_GRID_SIZE;
    uint16_t y_min = tmp_clamp((uint16_t)(b->position.y - TMP_BALL_RADIUS), 0,
                               TMP_MAP_SIZE - 1) /
                     TMP_SPATIAL_HASH_GRID_SIZE;
    uint16_t x_max = tmp_clamp((uint16_t)(b->position.x + TMP_BALL_RADIUS), 0,
                               TMP_MAP_SIZE - 1) /
                     TMP_SPATIAL_HASH_GRID_SIZE;
    uint16_t y_max = tmp_clamp((uint16_t)(b->position.y + TMP_BALL_RADIUS), 0,
                               TMP_MAP_SIZE - 1) /
                     TMP_SPATIAL_HASH_GRID_SIZE;

    for (uint16_t x = x_min; x <= x_max; x++)
        for (uint16_t y = y_min; y <= y_max; y++)
        {
            int const hash = HASH_FUNCTION(x, y);
            struct tmp_naive_hash_grid_bucket *bucket = m->buckets + hash;

            tmp_vector_grow(struct tmp_ball const *, bucket->items);
            *bucket->items++ = b;
        }
}

void tmp_naive_hash_grid_entity_from_ball(struct tmp_naive_hash_grid_entity *e,
                                          struct tmp_ball const *b)
{
    uint16_t x_min = tmp_clamp((uint16_t)(b->position.x - TMP_BALL_RADIUS), 0,
                               TMP_MAP_SIZE - 1) /
                     TMP_SPATIAL_HASH_GRID_SIZE;
    uint16_t y_min = tmp_clamp((uint16_t)(b->position.y - TMP_BALL_RADIUS), 0,
                               TMP_MAP_SIZE - 1) /
                     TMP_SPATIAL_HASH_GRID_SIZE;
    uint16_t x_max = tmp_clamp((uint16_t)(b->position.x + TMP_BALL_RADIUS), 0,
                               TMP_MAP_SIZE - 1) /
                     TMP_SPATIAL_HASH_GRID_SIZE;
    uint16_t y_max = tmp_clamp((uint16_t)(b->position.y + TMP_BALL_RADIUS), 0,
                               TMP_MAP_SIZE - 1) /
                     TMP_SPATIAL_HASH_GRID_SIZE;
    e->id = b->id;

    e->x_min = x_min;
    e->x_max = x_max;
    e->y_min = y_min;
    e->y_max = y_max;
}

static void update_implementation(struct tmp_naive_hash_grid *m)

    void tmp_naive_hash_grid_update_multiple(
        struct tmp_naive_hash_grid *m,
        struct tmp_naive_hash_grid_entity const *begin,
        struct tmp_naive_hash_grid_entity const *end)
{
    for (struct tmp_naive_hash_grid_entity const *i = begin; i < end; i++)
        update_implementation(m, *i);
}
#endif
