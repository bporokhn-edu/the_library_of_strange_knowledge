#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <cassert>

struct Data
{
    // The counters are aligned to 64 bytes (alignas(64)) to prevent false sharing.
    alignas(64) std::atomic<int> counter1;
    alignas(64) std::atomic<int> counter2;
};

void increment_counter(std::atomic<int> &counter)
{
    for (int i = 0; i < 1000000; ++i)
    {
        counter.fetch_add(1);
    }
}

// Define a structure with alignment to cache line size
struct alignas(std::hardware_destructive_interference_size) PaddedAtomic
{
    std::atomic<int> value;
};

// Define a structure with alignment to cache line size
struct alignas(std::hardware_constructive_interference_size) PaddedAtomicCo
{
    std::atomic<int> value;
};

void worker(PaddedAtomic *data, int thread_index, int num_iterations)
{
    for (int i = 0; i < num_iterations; ++i)
    {
        data[thread_index].value++;
    }
}

int main()
{

    {
        std::cout << "std::hardware_constructive_interference_size = " << std::hardware_constructive_interference_size << ", PaddedAtomicCo size of = " << sizeof(PaddedAtomicCo) << std::endl;
        std::cout << "std::hardware_destructive_interference_size = " << std::hardware_destructive_interference_size << ", PaddedAtomic size of = " << sizeof(PaddedAtomic) << std::endl;
    }
    {
        Data data{0, 0};

        auto start_time = std::chrono::high_resolution_clock::now();

        std::thread t1(increment_counter, std::ref(data.counter1));
        std::thread t2(increment_counter, std::ref(data.counter2));

        t1.join();
        t2.join();

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;

        std::cout << "Counter1: " << data.counter1.load() << std::endl;
        std::cout << "Counter2: " << data.counter2.load() << std::endl;
        std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
    }

    {
        // std::hardware_destructive_interference_size
        //      To prevent false sharing between threads.
        //      Minimum offset between two objects to avoid false sharing. Guaranteed to be at least alignof(std::max_align_t)
        // std::hardware_constructive_interference_size
        //      To promote true sharing between threads.
        //      Maximum size of contiguous memory to promote true sharing. Guaranteed to be at least alignof(std::max_align_t)

        const int num_threads = 4;
        const int num_iterations = 1000000;

        // Create an array of PaddedAtomic structures
        std::vector<PaddedAtomic> data(num_threads);

        // Create and run threads
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back(worker, data.data(), i, num_iterations);
        }

        // Join threads
        for (auto &t : threads)
        {
            t.join();
        }

        // Verify results
        for (int i = 0; i < num_threads; ++i)
        {
            assert(data[i].value == num_iterations);
            std::cout << "Thread " << i << ": " << data[i].value.load() << std::endl;
        }
    }
    return 0;
}
