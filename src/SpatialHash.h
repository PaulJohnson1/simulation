#pragma once

#include <stdint.h>

#include <Utilities.h>
#include <Const.h>

#define TMP_SPATIAL_HASH_CELL_COUNT_AXIS (TMP_MAP_SIZE / TMP_SPATIAL_HASH_GRID_SIZE)
#define TMP_SPATIAL_HASH_CELL_COUNT ((TMP_SPATIAL_HASH_CELL_COUNT_AXIS + 1) * (TMP_SPATIAL_HASH_CELL_COUNT_AXIS + 1))

struct tmp_simulation;
struct tmp_ball;

struct tmp_spatial_hash
{
    uint32_t cells[TMP_SPATIAL_HASH_CELL_COUNT];
    struct reference *references;
    uint64_t references_size;
    uint64_t free_reference;
};

void tmp_spatial_hash_init(struct tmp_spatial_hash *);
void tmp_spatial_hash_insert(struct tmp_spatial_hash *, struct tmp_ball *);
void tmp_spatial_hash_update(struct tmp_spatial_hash *, struct tmp_ball *);
void tmp_spatial_hash_optimize(struct tmp_spatial_hash *);
#define tmp_spatial_hash_find_possible_collisions tmp_spatial_hash_find_possible_collisions_single
void tmp_spatial_hash_find_possible_collisions_single(struct tmp_spatial_hash *, void *user_captures, void (*cb)(uint64_t, uint64_t, void *));
void tmp_spatial_hash_find_possible_collisions_multi(struct tmp_spatial_hash *, void *user_captures, void (*cb)(uint64_t, uint64_t, void *));