#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

// deepseek generated function
template <typename T>
void benchmark(std::string label, T x, float time_seconds = 10)
{
    using namespace std::chrono;

    struct comma_numpunct : std::numpunct<char>
    {
        char do_thousands_sep() const override { return ','; }
        std::string do_grouping() const override { return "\03"; }
    };
    std::locale comma_locale(std::locale(), new comma_numpunct());
    std::cout.imbue(comma_locale);

    const auto start_time = steady_clock::now();
    const auto target_duration =
        duration_cast<nanoseconds>(duration<float>(time_seconds));

    int64_t total_iterations = 0;
    int64_t batch_size = 1;

    std::cout << "Benchmarking " << std::quoted(label) << " for ~"
              << time_seconds << "s:\n";

    while (true)
    {
        std::cout << "  Batch size: " << std::setw(12) << batch_size
                  << " | Running... ";
        auto batch_start = steady_clock::now();

        for (int64_t i = 0; i < batch_size; ++i)
        {
            x(i + total_iterations);
            asm volatile("" : : : "memory");
        }

        auto batch_end = steady_clock::now();
        total_iterations += batch_size;

        const float batch_time =
            duration<float>(batch_end - batch_start).count();
        const float total_time =
            duration<float>(batch_end - start_time).count();

        std::cout << "took " << std::fixed << std::setprecision(6) << batch_time
                  << "s | Total: " << total_time << "s\n";

        if (total_time >= time_seconds)
            break;

        if (batch_time < time_seconds * 0.01f)
            batch_size *= 2;
        else
            batch_size = std::max<int64_t>(1, batch_size * 1.5f);
    }

    const auto final_start = steady_clock::now();
    const float measured_time =
        duration<float>(final_start - start_time).count();
    const float remaining_time = std::max(0.0f, time_seconds - measured_time);

    if (remaining_time > 0)
    {
        const int64_t final_batch_size = static_cast<int64_t>(
            total_iterations * remaining_time / measured_time);
        std::cout << "  Final batch: " << std::setw(12) << final_batch_size
                  << " | Running... ";

        auto batch_start = steady_clock::now();
        for (int64_t i = 0; i < final_batch_size; ++i)
        {
            x(total_iterations + i);
            asm volatile("" : : : "memory");
        }
        auto batch_end = steady_clock::now();

        total_iterations += final_batch_size;
        std::cout << "took " << std::setprecision(2)
                  << duration<float>(batch_end - batch_start).count() << "s\n";
    }

    const float total_elapsed =
        duration<float>(steady_clock::now() - start_time).count();
    const float ops_per_ms = total_iterations / total_elapsed / 1000.0;
    float us_per_ops = total_elapsed / total_iterations * 1'000'000;

    std::cout << "\nResults:\n"
              << "  Operations: " << total_iterations << "\n"
              << "  Time:       " << std::setprecision(2) << total_elapsed
              << "s\n"
              << "  Throughput: " << std::setprecision(2) << ops_per_ms
              << " ops/ms\n"
              << "  us/op       " << std::setprecision(4) << us_per_ops << '\n';

    std::cout.imbue(std::locale());
}

int main()
{
    printf("sizeof simulation: %lu\n", sizeof(struct tmp_simulation));

    struct tmp_simulation sim = {0};
    tmp_simulation_init(&tmp_simulation);

    benchmark(
        "simulation with 10 subticks",
        [&](uint64_t index)
        {
            if (index % 10 == 0)
                tmp_spatial_hash_optimize(&tmp_simulation.grid);

            tmp_simulation_subtick(&tmp_simulation, 0.1f);
        },
        10.0);
}
