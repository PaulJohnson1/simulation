#ifdef TMP_USE_HASH_GRID
#include <Collision/HashGrid.h>

#include <stdio.h>
#include <stdlib.h>

#include <Ball.h>
#include <Const.h>
#include <Simulation.h>
#include <string.h>

#define HASH_FUNCTION_FACTOR TMP_SPATIAL_HASH_CELL_COUNT_AXIS
#define HASH_FUNCTION(X, Y) ((X) + HASH_FUNCTION_FACTOR * (Y))

#define ll_size (TMP_BALL_COUNT)

struct references
{
    uint32_t datas[TMP_BALL_COUNT];
    uint32_t nexts[TMP_BALL_COUNT];
};

void tmp_spatial_hash_init(struct tmp_spatial_hash *g)
{
    memset(g, 0, sizeof *g);
    g->references = (struct references *)calloc(1, sizeof *g->references);
    g->references_size = 1;
}

static uint32_t create_reference(struct tmp_spatial_hash *g)
{
    if (g->free_reference)
    {
        uint32_t r = g->free_reference;
        g->free_reference = g->references->nexts[r];
        return r;
    }

    return g->references_size++;
}

// uniform
void tmp_spatial_hash_insert(struct tmp_spatial_hash *g,
                             struct tmp_ball const *b)
{
    uint64_t x =
        (uint16_t)tmp_clamp((int64_t)b->position.x, 0, TMP_MAP_SIZE - 1) /
        TMP_SPATIAL_HASH_GRID_SIZE;
    uint64_t y =
        (uint64_t)tmp_clamp((int64_t)b->position.y, 0, TMP_MAP_SIZE - 1) /
        TMP_SPATIAL_HASH_GRID_SIZE;
    uint64_t hash = HASH_FUNCTION(x, y);
    g->current_entity_cells[b->id] = hash;
    uint32_t i = create_reference(g);
    g->references->datas[i] = b->id;
    g->references->nexts[i] = g->cells[hash];
    g->cells[hash] = i;
}

static void update_implementation(struct tmp_spatial_hash *g,
                                  struct tmp_spatial_hash_entity const entity)
{
    uint64_t new_hash = HASH_FUNCTION((uint64_t)entity.x, (uint64_t)entity.y);
    uint64_t old_hash = g->current_entity_cells[entity.id];
    if (new_hash == old_hash)
        return;

    g->current_entity_cells[entity.id] = new_hash;

    // delete
    uint32_t current = g->cells[old_hash];
    uint32_t prev = 0;
    while (current != 0)
    {
        if (g->references->datas[current] == entity.id)
        {
            if (prev == 0)
                g->cells[old_hash] = g->references->nexts[current];
            else
                g->references->nexts[prev] = g->references->nexts[current];
            break;
        }

        prev = current;
        current = g->references->nexts[current];
    }

    // insert
    // can reuse the same linked list element
    g->references->datas[current] = entity.id;
    g->references->nexts[current] = g->cells[new_hash];
    g->cells[new_hash] = current;
}

void tmp_spatial_hash_entity_from_ball(struct tmp_spatial_hash_entity *e,
                                       struct tmp_ball const *b)
{
    e->id = b->id;
    e->x = (uint16_t)tmp_clamp((int16_t)b->position.x, 0, TMP_MAP_SIZE - 1) /
           TMP_SPATIAL_HASH_GRID_SIZE;
    e->y = (uint16_t)tmp_clamp((int16_t)b->position.y, 0, TMP_MAP_SIZE - 1) /
           TMP_SPATIAL_HASH_GRID_SIZE;
}

void tmp_spatial_hash_update(struct tmp_spatial_hash *g,
                             struct tmp_ball const *b)
{
    struct tmp_spatial_hash_entity entity;
    tmp_spatial_hash_entity_from_ball(&entity, b);
    update_implementation(g, entity);
}

void tmp_spatial_hash_update_multiple(
    struct tmp_spatial_hash *g, struct tmp_spatial_hash_entity const *begin,
    struct tmp_spatial_hash_entity const *end)
{
    for (struct tmp_spatial_hash_entity const *i = begin; i < end; i++)
        update_implementation(g, *i);
}

static void print_grid(struct tmp_spatial_hash const *g)
{
    uint32_t const *end = g->cells + TMP_SPATIAL_HASH_CELL_COUNT;
    for (uint32_t const *cell = g->cells; cell < end; cell++)
    {
        if (!*cell)
            continue;
        printf("cell %ld  \ti: ", cell - g->cells);
        for (uint32_t j = *cell; j; j = g->references->nexts[j])
            printf("%u(%u),", j, g->references->datas[j]);
        puts("eof");
    }
}

void tmp_spatial_hash_optimize(struct tmp_spatial_hash *g)
{
    struct references *new_refs =
        (struct references *)calloc(1, sizeof *new_refs);

    uint32_t const *end = g->cells + TMP_SPATIAL_HASH_CELL_COUNT;
    uint32_t i = 1;
    for (uint32_t *cell = g->cells; cell < end; cell++)
    {
        if (!*cell)
            continue;
        uint32_t const entity_i = *cell;
        *cell = i;

        for (uint32_t j = entity_i; j; j = g->references->nexts[j])
        {
            new_refs->datas[i] = g->references->datas[j];
            new_refs->nexts[i] = i + 1;
            i++;
        }
        new_refs->nexts[i - 1] = 0;
    }

    free(g->references);
    g->references = new_refs;
    g->free_reference = 0;
}

void tmp_spatial_hash_find_possible_collisions_single(
    struct tmp_spatial_hash const *g, void *captures,
    void (*cb)(uint64_t, uint64_t, void *))
{
#define search(h)                                                              \
    do                                                                         \
    {                                                                          \
        for (uint32_t j = g->cells[h]; j; j = g->references->nexts[j])         \
            cb(id_a, g->references->datas[j], captures);                       \
    } while (0)
#define prefetch(h)                                                            \
    do                                                                         \
    {                                                                          \
        __builtin_prefetch(g->references->nexts + (h), 0, 0);                  \
        __builtin_prefetch(g->references->datas + (h), 0, 0);                  \
    } while (0)

    uint64_t const end = TMP_SPATIAL_HASH_CELL_COUNT_AXIS;
    for (uint64_t y = 0; y < end; y++)
    {
        for (uint64_t x = 0; x < end; x++)
        {
            uint64_t const hash = HASH_FUNCTION(x, y);
            uint32_t const cell = g->cells[hash];

            for (uint32_t i = cell; i; i = g->references->nexts[i])
            {
                uint64_t const id_a = g->references->datas[i];

                // middle
                for (uint32_t j = g->references->nexts[i]; j;
                     j = g->references->nexts[j])
                    cb(id_a, g->references->datas[j], captures);

                if (x < end - 1)
                    search(hash + 1); // right

                if (y > 0)
                {
                    search(hash - HASH_FUNCTION_FACTOR); // down
                    if (x > 0)
                        search(hash - HASH_FUNCTION_FACTOR - 1); // down left
                    if (x < end - 1)
                        search(hash - HASH_FUNCTION_FACTOR + 1); // down right
                }
            }
        }
    }
}
#endif
