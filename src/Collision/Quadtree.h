#pragma once

#include <stdint.h>

#include <Utilities.h>

struct tmp_ball;
struct tmp_simulation;

struct tmp_quadtree_entity
{
    struct tmp_quadtree_node *node;
    int32_t x;
    int32_t y;
    uint32_t id;
};

struct tmp_quadtree_bounds
{
    int32_t x_min;
    int32_t x_max;
    int32_t y_min;
    int32_t y_max;
};

struct tmp_quadtree_node
{
    struct tmp_quadtree_bounds bounds;
    union
    {
        struct tmp_quadtree_node *nodes;
        struct
        {
            tmp_vector_declare(uint32_t, entities);
        };
    };
    uint8_t divided;
};

struct tmp_quadtree
{
    struct tmp_quadtree_node root;
    struct tmp_simulation *sim;
};

extern void tmp_quadtree_init(struct tmp_quadtree *);
extern void tmp_quadtree_insert(struct tmp_quadtree *, struct tmp_ball const *);
extern void tmp_quadtree_entity_from_ball(struct tmp_quadtree_entity *,
                                          struct tmp_ball const *);
extern void
tmp_quadtree_update_multiple(struct tmp_quadtree *,
                             struct tmp_quadtree_entity const *begin,
                             struct tmp_quadtree_entity const *end);
extern void tmp_quadtree_find_possible_collisions(struct tmp_quadtree const *,
                                                  void const *user_captures,
                                                  void (*cb)(uint64_t, uint64_t,
                                                             void const *));
