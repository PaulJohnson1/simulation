#pragma once

#include <stdint.h>

struct tmp_ball;
struct tmp_simulation;

struct tmp_brute_force_entity
{
};

struct tmp_brute_force
{
    struct tmp_simulation *sim;
};

extern void tmp_brute_force_find_possible_collisions(
    struct tmp_brute_force const *, void const *user_captures,
    void (*cb)(uint64_t, uint64_t, void const *));
