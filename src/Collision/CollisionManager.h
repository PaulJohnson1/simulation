#pragma once

#ifdef TMP_USE_HASH_GRID
#define TMP_FOUND_COLLISION_MANAGER
#include <Collision/HashGrid.h>
#define tmp_collision_manager tmp_spatial_hash
#define tmp_collision_manager_entity tmp_spatial_hash_entity
#define tmp_collision_manager_init tmp_spatial_hash_init
#define tmp_collision_manager_entity_from_ball tmp_spatial_hash_entity_from_ball
#define tmp_collision_manager_update_multiple tmp_spatial_hash_update_multiple
#define tmp_collision_manager_find_possible_collisions                         \
    tmp_spatial_hash_find_possible_collisions
#define tmp_collision_manager_optimize tmp_spatial_hash_optimize
#define tmp_collision_manager_insert tmp_spatial_hash_insert
#elif TMP_USE_BRUTE_FORCE
#define TMP_FOUND_COLLISION_MANAGER
#include <Collision/BruteForce.h>
#define tmp_collision_manager tmp_brute_force
#define tmp_collision_manager_entity tmp_brute_force_entity
#define tmp_collision_manager_init(...)
#define tmp_collision_manager_insert(...)
#define tmp_collision_manager_entity_from_ball(...)
#define tmp_collision_manager_update_multiple(...)
#define tmp_collision_manager_find_possible_collisions                         \
    tmp_brute_force_find_possible_collisions
#define tmp_collision_manager_optimize(...)
#elif TMP_USE_QUADTREE
#define TMP_FOUND_COLLISION_MANAGER
#include <Collision/Quadtree.h>
#define tmp_collision_manager tmp_quadtree
#define tmp_collision_manager_entity tmp_quadtree_entity
#define tmp_collision_manager_init tmp_quadtree_init
#define tmp_collision_manager_insert tmp_quadtree_insert
#define tmp_collision_manager_entity_from_ball tmp_quadtree_entity_from_ball
#define tmp_collision_manager_update_multiple tmp_quadtree_update_multiple
#define tmp_collision_manager_find_possible_collisions                         \
    tmp_quadtree_find_possible_collisions
#define tmp_collision_manager_optimize(...)
#elif TMP_USE_NAIVE_HASH_GRID
#define TMP_FOUND_COLLISION_MANAGER
#include <Collision/NaiveHashGrid.h>
#define tmp_collision_manager tmp_naive_hash_grid
#define tmp_collision_manager_entity tmp_naive_hash_grid_entity
#define tmp_collision_manager_init tmp_naive_hash_grid_init
#define tmp_collision_manager_insert tmp_naive_hash_grid_insert
#define tmp_collision_manager_entity_from_ball                                 \
    tmp_naive_hash_grid_entity_from_ball
#define tmp_collision_manager_update_multiple                                  \
    tmp_naive_hash_grid_update_multiple
#define tmp_collision_manager_find_possible_collisions(...)
#define tmp_collision_manager_optimize(...)

#endif

#ifndef TMP_FOUND_COLLISION_MANAGER
#error no tmp_collision_manager was found. define TMP_USE_HASH_GRID
#endif
