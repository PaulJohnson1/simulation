#ifdef TMP_USE_QUADTREE
#include <Collision/Quadtree.h>

#include <string.h>

#include <Ball.h>
#include <Const.h>
#include <Simulation.h>
#include <Utilities.h>

#define QUADTREE_MIN_SIZE 1
#define QUADTREE_SPLIT 10

static int rect_intersects_rect(struct tmp_quadtree_bounds const *restrict a,
                                struct tmp_quadtree_bounds const *restrict b)
{
    if (a->x_max < b->x_min)
        return 0;
    if (a->x_min > b->x_max)
        return 0;
    if (a->y_max < b->y_min)
        return 0;
    if (a->y_min < b->y_max)
        return 0;

    return 1;
}

void tmp_quadtree_init(struct tmp_quadtree *m)
{
    memset(m, 0, sizeof *m);
    m->root.bounds.x_min = 0;
    m->root.bounds.y_max = TMP_MAP_SIZE;
    m->root.bounds.x_min = 0;
    m->root.bounds.y_max = TMP_MAP_SIZE;
}

static struct tmp_quadtree_node *nw(struct tmp_quadtree_node const *n)
{
    return n->nodes + 0;
}

static struct tmp_quadtree_node *ne(struct tmp_quadtree_node const *n)
{
    return n->nodes + 1;
}

static struct tmp_quadtree_node *sw(struct tmp_quadtree_node const *n)
{
    return n->nodes + 2;
}

static struct tmp_quadtree_node *se(struct tmp_quadtree_node const *n)
{
    return n->nodes + 3;
}

static struct tmp_quadtree_node *get_quadrant(struct tmp_quadtree_node *node,
                                              int n, int w)
{
    if (w)
    {
        if (n)
            return nw(node);
        return sw(node);
    }
    if (n)
        return ne(node);
    return se(node);
}

static int32_t bounds_x_mid(struct tmp_quadtree_bounds const *b)
{
    return (b->x_max - b->x_min) / 2;
}

static int32_t bounds_y_mid(struct tmp_quadtree_bounds const *b)
{
    return (b->y_max - b->y_min) / 2;
}

static void node_init(struct tmp_quadtree_node *n) { memset(n, 0, sizeof *n); }

static void split(struct tmp_quadtree_node *n)
{
    n->divided = 1;
    struct tmp_quadtree_node *new_nodes = malloc(4 * sizeof *new_nodes);
    n->nodes = new_nodes;
    int32_t x_min = n->bounds.x_min;
    int32_t x_max = n->bounds.x_max;
    int32_t y_min = n->bounds.y_min;
    int32_t y_max = n->bounds.y_max;
    int32_t x_half = bounds_x_mid(&n->bounds);
    int32_t y_half = bounds_y_mid(&n->bounds);
    int32_t x_mid = x_half + x_min;
    int32_t y_mid = y_half + y_min;

    *nw(n) = (struct tmp_quadtree_node){.bounds = {x_min, x_mid, y_mid, y_max},
                                        .divided = 0};
    *ne(n) = (struct tmp_quadtree_node){.bounds = {x_mid, x_max, y_mid, y_max},
                                        .divided = 0};
    *sw(n) = (struct tmp_quadtree_node){.bounds = {x_min, x_mid, y_min, y_mid},
                                        .divided = 0};
    *se(n) = (struct tmp_quadtree_node){.bounds = {x_mid, x_max, y_min, y_mid},
                                        .divided = 0};
}

static void insert(struct tmp_quadtree *qt, struct tmp_quadtree_node *node,
                   int32_t x, int32_t y, uint32_t id)
{
    if (node->divided)
    {
        int n = y < bounds_y_mid(&node->bounds);
        int w = y < bounds_x_mid(&node->bounds);

        // todo: remove this disgusting recursion
        insert(qt, get_quadrant(node, n, w), x, y, id);

        return;
    }

    if (tmp_vector_size(node->entities) > QUADTREE_SPLIT)
    {
        split(node);
        int64_t s = tmp_vector_size(node->entities);
        for (int64_t i = 0; i < s; i++)
        {
            struct tmp_quadtree_entity *e = qt->sim->collision_entities + i;
            insert(qt, node, e->x, e->y, e->id);
        }
        insert(qt, node, x, y, id);
        return;
    }

    tmp_vector_grow(uint32_t, node->entities);
    *node->entities_end++ = id;
}

struct query_captures
{
    void const *user_captures;
    uint64_t id_a;
};

static void query(struct tmp_quadtree_node const *node,
                  struct tmp_quadtree_bounds const *bounds,
                  void const *_captures,
                  void (*cb)(uint64_t, uint64_t, void const *))
{
    if (node->divided)
    {
        // todo: remove this terrible recursion
        if (rect_intersects_rect(bounds, &nw(node)->bounds))
            query(nw(node), bounds, _captures, cb);
        if (rect_intersects_rect(bounds, &ne(node)->bounds))
            query(ne(node), bounds, _captures, cb);
        if (rect_intersects_rect(bounds, &sw(node)->bounds))
            query(sw(node), bounds, _captures, cb);
        if (rect_intersects_rect(bounds, &se(node)->bounds))
            query(se(node), bounds, _captures, cb);
        return;
    }

    struct query_captures const *captures = _captures;
    for (uint32_t *i = node->entities; i < node->entities_end; i++)
        cb(captures->id_a, *i, captures->user_captures);
}

void tmp_quadtree_insert(struct tmp_quadtree *m, struct tmp_ball const *b)
{
    insert(m, &m->root, (int32_t)b->position.x, (int32_t)b->position.y, b->id);
}

void tmp_quadtree_find_possible_collisions(struct tmp_quadtree const *qt,
                                           void const *user_captures,
                                           void (*cb)(uint64_t, uint64_t,
                                                      void const *))
{
    for (struct tmp_ball *b = qt->sim->balls; b < qt->sim->balls_end; b++)
    {
        struct tmp_quadtree_bounds bounds = {
            .x_min = (int32_t)(b->position.x - TMP_BALL_RADIUS),
            .x_max = (int32_t)(b->position.x + TMP_BALL_RADIUS),
            .y_min = (int32_t)(b->position.y - TMP_BALL_RADIUS),
            .y_max = (int32_t)(b->position.y + TMP_BALL_RADIUS),
        };
        struct query_captures captures = {
            .user_captures = user_captures,
            .id_a = (uint64_t)(qt->sim->balls_end - b)};
        query(&qt->root, &bounds, &captures, cb);
    }
}
#endif
