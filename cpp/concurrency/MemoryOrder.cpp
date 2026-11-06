#include <iostream>
#include <fstream>
#include <exception>
#include <chrono>
#include <mutex>
#include <pthread.h>
#include <cstring>
#include <thread>
#include <sstream>
#include <atomic>
#include <type_traits>

/*
1. std::memory_order_relaxed
    Description: This memory order imposes no synchronization or ordering constraints.
    Use Case: Use it when you don't need any ordering guarantees, only atomicity.
    Example: Counting occurrences in multiple threads where the exact order of updates does not matter.
2. std::memory_order_consume
    Description: This is a weaker form of std::memory_order_acquire. It ensures that dependent loads are not reordered before the load.
    Use Case: It's rarely used due to its complexity and the fact that most compilers treat it as std::memory_order_acquire.
    Example: If thread A writes to an atomic variable and thread B reads it, all subsequent reads in B that are dependent on the atomic read will see the writes from A.
3. std::memory_order_acquire
    Description: Ensures that all memory writes before the acquire operation become visible to all operations after it.
    Use Case: Used when a thread needs to read a value and then see all the writes made by the thread that wrote the value.
    Example: Reading a flag to ensure all preceding writes are visible to the current thread.
4. std::memory_order_release
    Description: Ensures that all memory writes in the current thread are visible to all other threads performing an acquire operation.
    Use Case: Used when a thread needs to publish a value to be seen by other threads.
    Example: Writing to a flag to ensure all preceding writes are visible to another thread that reads the flag.
5. std::memory_order_acq_rel
    Description: Combines the effects of memory_order_acquire and memory_order_release. Ensures all writes before the release are visible to the acquiring operations, and no read/write reordering is allowed around the operation.
    Use Case: Used when an atomic operation both reads and writes a value.
    Example: An atomic read-modify-write operation like fetch_add.
6. std::memory_order_seq_cst
    Description: Provides the strongest memory ordering. All operations appear to happen in a single, globally ordered sequence. This is the default and most intuitive ordering.
    Use Case: Use when you need a simple and strong guarantee of ordering across threads.
    Example: Ensuring a consistent view of shared variables across all threads.
*/

int main()
{
    std::cout << "Atomc memory order start" << std::endl;

    std::cout << "\nmemory_order_relaxed" << std::endl;
    /*
    - No synchronization or ordering constraints are imposed on other reads or writes.

    - Use it when you only care about atomicity, but not about ordering.
    */
    {
        std::atomic<int> counter(0);
        auto increment = [&counter]()
        {
            for (int i = 0; i < 1000; ++i)
            {
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        };

        std::thread t1(increment);
        std::thread t2(increment);

        t1.join();
        t2.join();

        std::cout << "Counter: " << counter.load(std::memory_order_relaxed) << std::endl;
    }

    std::cout << "\nmemory_order_consume" << std::endl;
    /*
    (not widely supported, typically behaves like memory_order_acquire)

    A relaxed load followed by dependent operations (e.g., reading a pointer and dereferencing it)
    ensures those dependent operations see the effects of previous stores.
    */
    {
        std::atomic<int *> ptr(nullptr);
        int data;

        auto producer = [&data, &ptr]()
        {
            std::cout << "producer" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            data = 42;
            ptr.store(&data, std::memory_order_release);
            std::cout << "producer end " << std::endl;
        };

        auto consumer = [&data, &ptr]()
        {
            std::cout << "consumer" << std::endl;
            int *p;
            while (!(p = ptr.load(std::memory_order_consume)))
            {
            }
            // p-> is dependent on the load, so data should be 42
            std::cout << "Data: " << *p << std::endl;
        };
        std::thread t1(producer);
        std::thread t2(consumer);

        t1.join();
        t2.join();
    }

    std::cout << "\nmemory_order_acquire" << std::endl;
    /*
    No reads or writes in the current thread can be reordered before this load.

    Ensures that subsequent operations see the effects of operations in other threads that release.
    */
    {
        std::atomic<int *> ptr(nullptr);
        int data;

        auto producer = [&data, &ptr]()
        {
            std::cout << "producer" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            data = 42;
            ptr.store(&data, std::memory_order_release);
            std::cout << "producer end " << std::endl;
        };

        auto consumer = [&data, &ptr]()
        {
            std::cout << "consumer" << std::endl;
            while (!ptr.load(std::memory_order_acquire))
            {
            }
            // data should be 42
            std::cout << "Data: " << *ptr << std::endl;
        };
        std::thread t1(producer);
        std::thread t2(consumer);

        t1.join();
        t2.join();
    }
    std::cout << "\nmemory_order_release" << std::endl;
    /*
    No reads or writes in the current thread can be reordered after this store.

    Ensures that previous operations are visible to other threads that acquire.
    */
    {
        std::atomic<int *> ptr(nullptr);
        int data;

        auto producer = [&data, &ptr]()
        {
            std::cout << "producer" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            data = 42;
            ptr.store(&data, std::memory_order_release);
            std::cout << "producer end " << std::endl;
        };

        auto consumer = [&data, &ptr]()
        {
            std::cout << "consumer" << std::endl;
            while (!ptr.load(std::memory_order_acquire))
            {
            }
            // data should be 42
            std::cout << "Data: " << *ptr << std::endl;
        };
        std::thread t1(producer);
        std::thread t2(consumer);

        t1.join();
        t2.join();
    }

    std::cout << "\nmemory_order_acq_rel" << std::endl;
    /*
    Combines the effects of both acquire and release.

    Useful for read-modify-write operations to ensure proper synchronization.
    */
    {
        std::atomic<int> counter(0);
        auto increment = [&counter]()
        {
            std::cout << "increment" << std::endl;
            // std::this_thread::sleep_for(std::chrono::seconds(1));
            for (int i = 0; i < 1000000; ++i)
            {
                counter.fetch_add(1, std::memory_order_acq_rel);
            }
            std::cout << "increment end" << std::endl;
        };

        std::thread t1(increment);
        std::thread t2(increment);

        t1.join();
        // Ensures counter >= 1000
        std::cout << "Counter: " << counter.load(std::memory_order_acquire) << std::endl;
        t2.join();

        std::cout << "Counter: " << counter.load(std::memory_order_acquire) << std::endl;
    }

    std::cout << "\nmemory_order_seq_cst" << std::endl;
    /*
    The strongest memory order: sequential consistency.

    Operations appear to be executed in some global order.

    Ensures all previous operations are visible and ordered correctly.
    */
    {
        std::atomic<int> counter(0);
        auto increment = [&counter]()
        {
            std::cout << "increment" << std::endl;
            // std::this_thread::sleep_for(std::chrono::seconds(1));
            for (int i = 0; i < 1000000; ++i)
            {
                counter.fetch_add(1, std::memory_order_seq_cst);
            }
            std::cout << "increment end" << std::endl;
        };

        std::thread t1(increment);
        std::thread t2(increment);

        t1.join();
        // Ensures counter >= 1000
        std::cout << "Counter: " << counter.load(std::memory_order_seq_cst) << std::endl;
        t2.join();

        std::cout << "Counter: " << counter.load(std::memory_order_seq_cst) << std::endl;
    }

    std::cout << "\nstd::kill_dependency" << std::endl;
    {
        std::atomic<int *> atomic_ptr;
        {
            int *p = atomic_ptr.load(std::memory_order_consume);
            int value = *p; // Compiler assumes a dependency chain from load to this dereference
        }

        int *p = atomic_ptr.load(std::memory_order_consume);
        int *q = std::kill_dependency(p); // Inform the compiler there's no real dependency
        int value = *q;                   // No unnecessary memory order constraints
    }

    std::cout << std::endl
              << "Atomc memory order end" << std::endl;

    return 0;
}