#include <SpatialHash.h>

#include <Simulation.h>
#include <Ball.h>
#include <Bitset.h>
#include <string.h>

#define HASH_FUNCTION_FACTOR TMP_SPATIAL_HASH_CELL_COUNT_AXIS
#define HASH_FUNCTION(X, Y) ((X) + HASH_FUNCTION_FACTOR * (Y))

#define ll_size (1000000)

struct reference
{
    uint32_t next;
    uint32_t data;
};

void tmp_spatial_hash_init(struct tmp_spatial_hash *g)
{
    memset(g, 0, sizeof *g);
    g->references = calloc(ll_size, sizeof *g->references);
    g->references_size = 1;
    for (uint64_t i = 0; i < TMP_SPATIAL_HASH_CELL_COUNT; i++)
        g->cells[i] = 0;
}

static uint64_t create_reference(struct tmp_spatial_hash *g)
{
    if (g->free_reference)
    {
        uint64_t r = g->free_reference;
        g->free_reference = g->references[r].next;
        return r;
    }

    return g->references_size++;
}

// uniform
void tmp_spatial_hash_insert(struct tmp_spatial_hash *g, struct tmp_ball *b)
{
    uint16_t x = tmp_clamp((int16_t)b->position.x, 0, TMP_MAP_SIZE - 1) / TMP_SPATIAL_HASH_GRID_SIZE;
    uint16_t y = tmp_clamp((int16_t)b->position.y, 0, TMP_MAP_SIZE - 1) / TMP_SPATIAL_HASH_GRID_SIZE;
    uint64_t hash = HASH_FUNCTION(x, y);
    b->current_shg_cell = hash;
    uint32_t i = create_reference(g);
    g->references[i].data = b->id;
    g->references[i].next = g->cells[hash];
    g->cells[hash] = i;
}

void tmp_spatial_hash_update(struct tmp_spatial_hash *g, struct tmp_ball *b)
{
    uint16_t new_x = tmp_clamp((int16_t)b->position.x, 0, TMP_MAP_SIZE - 1) / TMP_SPATIAL_HASH_GRID_SIZE;
    uint16_t new_y = tmp_clamp((int16_t)b->position.y, 0, TMP_MAP_SIZE - 1) / TMP_SPATIAL_HASH_GRID_SIZE;
    uint64_t new_hash = HASH_FUNCTION(new_x, new_y);
    uint64_t old_hash = b->current_shg_cell;

    b->current_shg_cell = new_hash;

    if (new_hash == old_hash)
        return;

    // delete
    uint64_t current = g->cells[old_hash];
    uint64_t prev = 0;
    while (current != 0)
    {
        if (g->references[current].data == b->id)
        {
            if (prev == 0)
                g->cells[old_hash] = g->references[current].next;
            else
                g->references[prev].next = g->references[current].next;
            break;
        }

        prev = current;
        current = g->references[current].next;
    }

    // insert
    // can reuse the same linked list element
    g->references[current].data = b->id;
    g->references[current].next = g->cells[new_hash];
    g->cells[new_hash] = current;
}

static void print_grid(struct tmp_spatial_hash *g)
{
    uint32_t *end = g->cells + TMP_SPATIAL_HASH_CELL_COUNT;
    for (uint32_t *cell = g->cells; cell < end; cell++)
    {
        if (!*cell)
            continue;
        printf("cell %lu  \ti: ", cell - g->cells);
        for (uint32_t j = *cell; j; j = g->references[j].next)
            printf("%d(%d),", j, g->references[j].data);
        puts("eof");
    }
}

void tmp_spatial_hash_optimize(struct tmp_spatial_hash *g)
{
    struct reference *new_refs = calloc(ll_size, sizeof *new_refs);

    uint32_t *end = g->cells + TMP_SPATIAL_HASH_CELL_COUNT;
    uint64_t i = 1;
    for (uint32_t *cell = g->cells; cell < end; cell++)
    {
        if (!*cell)
            continue;
        uint32_t entity_i = *cell;
        *cell = i;

        for (uint32_t j = entity_i; j; j = g->references[j].next)
        {
            new_refs[i].data = g->references[j].data;
            new_refs[i].next = i + 1;
            i++;
        }
        new_refs[i - 1].next = 0;
    }

    free(g->references);
    g->references = new_refs;
    g->free_reference = 0;
}

void tmp_spatial_hash_find_possible_collisions_single(struct tmp_spatial_hash *g, void *captures, void (*cb)(uint64_t, uint64_t, void *))
{
#define search(h)                                             \
    do                                                        \
    {                                                         \
        uint32_t cell = g->cells[h];                          \
        for (uint32_t j = cell; j; j = g->references[j].next) \
            cb(id_a, g->references[j].data, captures);        \
    } while (0)

    uint64_t end = TMP_SPATIAL_HASH_CELL_COUNT_AXIS;
    for (uint64_t y = 0; y < end; y++)
        for (uint64_t x = 0; x < end; x++)
        {
            uint64_t hash = HASH_FUNCTION(x, y);
            uint32_t cell = g->cells[hash];
            for (uint32_t i = cell; i; i = g->references[i].next)
            {
                uint64_t id_a = g->references[i].data;

                // middle
                for (uint32_t j = g->references[i].next; j; j = g->references[j].next)
                    cb(id_a, g->references[j].data, captures);

                if (x < end)
                    search(hash + 1); // right

                if (y > 0)
                {
                    if (x > 0)
                        search(hash - HASH_FUNCTION_FACTOR - 1); // down left
                    if (x < end)
                        search(hash - HASH_FUNCTION_FACTOR + 1); // down right
                    search(hash - HASH_FUNCTION_FACTOR);         // down
                }
            }
        }
}
