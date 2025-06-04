# High performance simulation
Handles 20,000 balls at 0.687 milliseconds per subtick on my cpu: `Intel(R) Core(TM) i3-1005G1 CPU @ 1.20GHz`
There are multiple subticks per tick. Each subtick includes the gravity, verlet integration, applying the map constraints, updating the collision manager, finding the collisions, and resolving the collisions.

## Buliding
to compile: `cc nob.c && ./a.out`
for more compile options (no rendering, debug, release, profile, pgo), check nob.c
