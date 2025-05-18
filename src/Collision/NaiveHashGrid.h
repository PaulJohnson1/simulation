#pragma once

#include <stdint.h>

#include <Const.h>
#include <Utilities.h>

#define TMP_NAIVE_HASH_GRID_CELL_COUNT_AXIS                                    \
    (TMP_MAP_SIZE / TMP_SPATIAL_HASH_GRID_SIZE)
#define TMP_NAIVE_HASH_GRID_CELL_COUNT                                         \
    ((TMP_NAIVE_HASH_GRID_CELL_COUNT_AXIS + 1) *                               \
     (TMP_NAIVE_HASH_GRID_CELL_COUNT_AXIS + 1))

struct tmp_ball;

struct tmp_naive_hash_grid_bucket
{
    tmp_vector_declare(struct tmp_ball const *, items);
};

struct tmp_naive_hash_grid_entity
{
    uint32_t id;
    uint16_t x_min;
    uint16_t x_max;
    uint16_t y_min;
    uint16_t y_max;
};

struct tmp_naive_hash_grid
{
    struct tmp_naive_hash_grid_bucket buckets[TMP_NAIVE_HASH_GRID_CELL_COUNT];
};

extern void tmp_naive_hash_grid_init(struct tmp_naive_hash_grid *);
extern void tmp_naive_hash_grid_insert(struct tmp_naive_hash_grid *,
                                       struct tmp_ball const *);
extern void
tmp_naive_hash_grid_entity_from_ball(struct tmp_naive_hash_grid_entity *,
                                     struct tmp_ball const *);
extern void tmp_naive_hash_grid_update_multiple(
    struct tmp_naive_hash_grid *,
    struct tmp_naive_hash_grid_entity const *begin,
    struct tmp_naive_hash_grid_entity const *end);
