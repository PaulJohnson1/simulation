#include <Simulation.h>
#include <cinttypes>
#include <iostream>
#include <string>
#include <sys/time.h>

uint64_t get_us_from_timeval(struct timeval x)
{
    return (uint64_t)(x.tv_sec * 1'000'000 + x.tv_usec);
}

template <typename T>
void benchmark(std::string label, T x, uint64_t time_useconds)
{
    (void)label;
    int64_t next_batch_size = 1;
    uint64_t total_iterations = 0;

    struct timeval benchmark_start_timeval;
    gettimeofday(&benchmark_start_timeval, NULL);
    uint64_t benchmark_start = get_us_from_timeval(benchmark_start_timeval);
    uint64_t benchmark_end = benchmark_start + time_useconds;

    while (true)
    {
        printf("running batch of size:\t%" PRIu64, next_batch_size);
        fflush(stdout);
        struct timeval start;
        gettimeofday(&start, NULL);
        for (int64_t i = 0; i < next_batch_size; i++)
        {
            x(total_iterations);
            total_iterations++;
        }
        struct timeval end;
        gettimeofday(&end, NULL);

        uint64_t time_elapsed =
            get_us_from_timeval(end) - get_us_from_timeval(start);
        printf(": took %" PRIu64 " us\n", time_elapsed);
        int64_t time_remaining =
            (int64_t)benchmark_end - (int64_t)get_us_from_timeval(end);
        // prevent divide by 0
        if (next_batch_size == 0)
            break;
        next_batch_size =
            time_remaining / ((int64_t)time_elapsed / next_batch_size);
        next_batch_size -= next_batch_size / 5;
        if (time_remaining < 0 || next_batch_size == 0)
            break;
    }

    struct timeval benchmark_end_timeval;
    gettimeofday(&benchmark_end_timeval, NULL);
    uint64_t benchmark_time_elapsed =
        get_us_from_timeval(benchmark_end_timeval) - benchmark_start;

    printf("results: \n"
           "\titerations: %lu\n"
           "\ttime:\t%lu\n"
           "\tus/op:\t%.3f\n"
           "\tops/ms:\t%.3f\n",
           total_iterations, benchmark_time_elapsed,
           (float)benchmark_time_elapsed / (float)total_iterations,
           (float)total_iterations / ((float)benchmark_time_elapsed / 1'000));
}

int main()
{
    printf("sizeof simulation: %lu\n", sizeof(struct tmp_simulation));

    struct tmp_simulation sim;
    tmp_simulation_init(&sim);

    // for (uint64_t i = 1; i < 1000; i *= 2)
    benchmark(
        "simulation",
        [&](uint64_t index)
        {
            (void)index;
            // if (index % 10 == 0)
            // tmp_collision_manager_optimize(&sim.collisions);

            tmp_simulation_subtick(&sim, 0.1f);
        },
        60'000'000);
}
