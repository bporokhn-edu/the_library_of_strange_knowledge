#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

/*
Weak vs. Strong:

Weak (atomic_compare_exchange_weak):
    May fail spuriously, meaning it can return false even if the expected value matches the current value.
    Useful in tight loops where performance is critical.
Strong (atomic_compare_exchange_strong):
    Guarantees no spurious failures, but might be slightly slower.

When using std::atomic_compare_exchange_weak_explicit or std::atomic_compare_exchange_strong_explicit, you can specify two different memory orderings:
success:
    This is the memory order that will be applied if the compare-and-swap (CAS) operation succeeds,
    i.e., if the current value in the atomic object matches the expected value, and the object is updated to the new value (desired).

failure:
    This is the memory order that will be applied if the CAS operation fails,
    i.e., if the current value in the atomic object does not match the expected value, and no update occurs.
    to update expexted var with the new value: load value

*/

class ThreadSafeCounter
{
public:
    ThreadSafeCounter() : counter(0) {}

    void increment()
    {
        int oldValue = counter.load(); // Load the current value of the counter
        int newValue;

        do
        {
            newValue = oldValue + 1; // Calculate the new value
            // Try to update the counter from oldValue to newValue
        } while (!counter.compare_exchange_weak(oldValue, newValue));
        // If compare_exchange_weak fails, oldValue is updated with the current value of the counter
        // The loop retries until the CAS operation succeeds
    }

    int get() const
    {
        return counter.load();
    }

private:
    std::atomic<int> counter;
};

void worker(ThreadSafeCounter &counter)
{
    for (int i = 0; i < 100000; ++i)
    {
        counter.increment(); // Increment the counter
    }
}

/*
is_lock_free():
    Runtime check.
    Can vary depending on the specific hardware, platform, and size of the atomic type.
    Provides information on whether the current instance of the atomic type is lock-free.

is_always_lock_free:
    Compile-time constant.
    Guarantees that the type is lock-free on all platforms where the code might run.
    Provides information on whether all instances of the atomic type are lock-free
*/

int main()
{

    std::atomic<int> atomicInt(1);
    if (atomicInt.is_lock_free())
    {
        std::cout << "The atomic<int> is lock-free." << std::endl;
    }
    else
    {
        std::cout << "The atomic<int> is not lock-free." << std::endl;
    }

    if (std::atomic<int>::is_always_lock_free)
    {
        std::cout << "The atomic<int> is always lock-free." << std::endl;
    }
    else
    {
        std::cout << "The atomic<int> might not be lock-free on some platforms." << std::endl;
    }

    std::atomic<int> atomicVar{10};
    int expected = 10;
    int desired = 50;

    bool result = std::atomic_compare_exchange_strong_explicit(&atomicVar, &expected, desired,
                                                               std::memory_order_acq_rel,
                                                               std::memory_order_relaxed);

    std::cout << "Result: " << result << ", New Value: " << atomicVar.load() << ", Expected: " << expected << std::endl
              << std::endl;

    ThreadSafeCounter counter;

    // Create multiple threads to increment the counter
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i)
    {
        threads.push_back(std::thread(worker, std::ref(counter)));
    }

    // Wait for all threads to finish
    for (auto &t : threads)
    {
        t.join();
    }

    // Print the final value of the counter
    std::cout << "Final Counter Value: " << counter.get() << std::endl;

    return 0;
}
