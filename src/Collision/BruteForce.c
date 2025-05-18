#include <Collision/BruteForce.h>

#include <Const.h>

void tmp_brute_force_find_possible_collisions(struct tmp_brute_force const *m,
                                              void const *user_captures,
                                              void (*cb)(uint64_t, uint64_t,
                                                         void const *))
{
    (void)m;
    for (uint64_t i = 0; i < TMP_BALL_COUNT; i++)
        for (uint64_t j = i + 1; j < TMP_BALL_COUNT; j++)
            cb(i, j, user_captures);
}
