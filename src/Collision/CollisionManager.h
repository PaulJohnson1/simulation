#pragma once

#ifdef TMP_USE_HASH_GRID
#define TMP_FOUND_COLLISION_MANAGER
#include <Collision/SpatialHash.h>
#define tmp_collision_manager tmp_spatial_hash
#define tmp_collision_manager_entity tmp_spatial_hash_entity
#define tmp_collision_manager_init tmp_spatial_hash_init
#define tmp_collision_manager_entity_from_ball tmp_spatial_hash_entity_from_ball
#define tmp_collision_manager_update_multiple tmp_spatial_hash_update_multiple
#define tmp_collision_manager_find_possible_collisions                         \
    tmp_spatial_hash_find_possible_collisions
#define tmp_collision_manager_optimize tmp_spatial_hash_optimize
#define tmp_collision_manager_insert tmp_spatial_hash_insert
#endif

#ifndef TMP_FOUND_COLLISION_MANAGER
#error no tmp_collision_manager was found. define TMP_USE_HASH_GRID
#endif
