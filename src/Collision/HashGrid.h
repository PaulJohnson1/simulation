#pragma once

#include <stdint.h>

#include <Const.h>
#include <Utilities.h>

#define TMP_SPATIAL_HASH_CELL_COUNT_AXIS                                       \
    (TMP_MAP_SIZE / TMP_SPATIAL_HASH_GRID_SIZE)
#define TMP_SPATIAL_HASH_CELL_COUNT                                            \
    ((TMP_SPATIAL_HASH_CELL_COUNT_AXIS) * (TMP_SPATIAL_HASH_CELL_COUNT_AXIS))

struct tmp_simulation;
struct tmp_ball;

struct tmp_spatial_hash
{
    struct references *references;
    struct tmp_simulation *sim;
    uint32_t references_size;
    uint32_t free_reference;
    // array of the current shg cell an entity occupies
    uint64_t current_entity_cells[TMP_BALL_COUNT];
    uint32_t cells[TMP_SPATIAL_HASH_CELL_COUNT];
};

struct tmp_spatial_hash_entity
{
    uint32_t id;
    uint16_t x;
    uint16_t y;
};

CPLUSPLUS_PREFIX
void tmp_spatial_hash_entity_from_ball(struct tmp_spatial_hash_entity *,
                                       struct tmp_ball const *);
void tmp_spatial_hash_init(struct tmp_spatial_hash *);
void tmp_spatial_hash_insert(struct tmp_spatial_hash *,
                             struct tmp_ball const *);
void tmp_spatial_hash_update(struct tmp_spatial_hash *,
                             struct tmp_ball const *);
void tmp_spatial_hash_update_multiple(
    struct tmp_spatial_hash *, struct tmp_spatial_hash_entity const *begin,
    struct tmp_spatial_hash_entity const *end);
void tmp_spatial_hash_optimize(struct tmp_spatial_hash *);
#define tmp_spatial_hash_find_possible_collisions                              \
    tmp_spatial_hash_find_possible_collisions_single
void tmp_spatial_hash_find_possible_collisions_single(
    struct tmp_spatial_hash const *, void *user_captures,
    void (*cb)(uint64_t, uint64_t, void *));
void tmp_spatial_hash_find_possible_collisions_multi(
    struct tmp_spatial_hash *, void *user_captures,
    void (*cb)(uint64_t, uint64_t, void *));
CPLUSPLUS_SUFFIX
