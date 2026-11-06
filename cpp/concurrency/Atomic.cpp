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
#include <bitset>

// Custom class
struct MyData
{
    int x_ = 0;
    float y_ = 0.0;

    MyData(int x = 0, double y = 0) : x_(x), y_(y) {}

    MyData operator+(const MyData &md)
    {
        return MyData(x_ + md.x_, y_ + md.y_);
    }

    MyData operator+(int i)
    {
        // will be printed
        // std::cout << "MyData operator+. x_ = " << x_ << ", y_ = " << y_ << ", i = " << i << std::endl;
        return MyData(x_ + i, y_ + i);
    }

    MyData increment(int i = 1)
    {
        return MyData(x_ + i, y_ + i);
    }

    // Trivial copy constructor, assignment operator, and destructor
};

// Specialize std::atomic for MyData is unused if MyData trivial copy, move, destruct
// template <>
// struct std::atomic<MyData>
// {
//     using value_type = MyData;

//     std::atomic<int> x;
//     std::atomic<double> y;

//     atomic() noexcept : x(0), y(0.0) {}
//     atomic(int x_, double y_) noexcept : x(x_), y(y_) {}

//     MyData load(std::memory_order order = std::memory_order_seq_cst) const noexcept
//     {
//         return MyData(x.load(order), y.load(order));
//     }

//     void store(MyData desired, std::memory_order order = std::memory_order_seq_cst) noexcept
//     {
//         x.store(desired.x, order);
//         y.store(desired.y, order);
//     }

//     bool compare_exchange_strong(MyData &expected, MyData desired, std::memory_order order = std::memory_order_seq_cst) noexcept
//     {
//         return x.compare_exchange_strong(expected.x, desired.x, order) &&
//                y.compare_exchange_strong(expected.y, desired.y, order);
//     }
// };

int main()
{
    std::cout << "Atomc start" << std::endl;

    {
        std::atomic<bool> atomicData(1);
        if (atomicData.is_lock_free())
        {
            std::cout << "The atomic<bool> is lock-free." << std::endl;
        }
        else
        {
            std::cout << "The atomic<bool> is not lock-free." << std::endl;
        }

        if (std::atomic<bool>::is_always_lock_free)
        {
            std::cout << "The atomic<bool> is always lock-free." << std::endl;
        }
        else
        {
            std::cout << "The atomic<bool> might not be lock-free on some platforms." << std::endl;
        }
    }
    std::cout << "\nCustom atomic data type" << std::endl;
    {
        auto increment = [](std::atomic<MyData> &data)
        {
            for (int i = 0; i < 1000; ++i)
            {
                MyData current = data.load();
                // lock-free algorithm
                while (!data.compare_exchange_weak(current, current + 1))
                    ;
            }
        };
        MyData md(1, 1.0);
        std::atomic<MyData> data(md);

        std::thread t1(increment, std::ref(data));
        std::thread t2(increment, std::ref(data));

        t1.join();
        t2.join();

        MyData finalData = data.load(std::memory_order_relaxed);
        std::cout << "Final data: x = " << finalData.x_ << ", y = " << finalData.y_ << std::endl;
    }

    std::cout << "\nStore, load, exchanged atomic" << std::endl;
    {
        std::atomic_int a(0);
        int load = a.load();
        std::cout << "a = " << load << std::endl;
        a.store(2);
        std::cout << "a = " << a << std::endl;

        int old = a.exchange(5);
        std::cout << "a = " << a << " old a = " << old << std::endl;
    }

    std::cout << "\nAtomic Fetch_*" << std::endl;
    {
        std::atomic<int> counter{10};
        int oldValue = atomic_fetch_add(&counter, 5); // counter becomes 15, oldValue is 10
        std::cout << "Counter = " << counter.load() << ", old value = " << oldValue << std::endl;

        oldValue = counter.fetch_sub(5); // counter becomes 10, oldValue is 15
        std::cout << "Counter = " << counter.load() << ", old value = " << oldValue << std::endl;

        std::atomic<int> flags{0b1100};               // Binary 1100
        oldValue = atomic_fetch_and(&flags, 0b01010); // flags becomes 0b1000, oldValue is 0b1100
        std::cout << "Flags = " << std::bitset<sizeof(int)>(flags.load()) << ", old value = " << std::bitset<sizeof(int)>(oldValue) << std::endl;

        std::atomic<int> flags_or{0b1001};    // Binary 1001
        oldValue = flags_or.fetch_or(0b0110); // flags becomes 0b1111, oldValue is 0b1001
        std::cout << "Flags = " << std::bitset<sizeof(int)>(flags_or.load()) << ", old value = " << std::bitset<sizeof(int)>(oldValue) << std::endl;

        std::atomic<int> flags_xor{0b1100};     // Binary 1100
        oldValue = flags_xor.fetch_xor(0b1010); // flags becomes 0b0110, oldValue is 0b1100
        std::cout << "Flags = " << std::bitset<sizeof(int)>(flags_xor.load()) << ", old value = " << std::bitset<sizeof(int)>(oldValue) << std::endl;

        // Since c++26
        // std::atomic<int> number{20};
        // oldValue = atomic_fetch_min(&number, 10); // number becomes 10, oldValue is 20
        // std::cout << "Number = " << number.load() << ", old value = " << oldValue << std::endl;

        // int oldValue = number.fetch_min(number, 21); // number becomes 21, oldValue is 20
        // std::cout << "Number = " << number.load() << ", old value = " << oldValue << std::endl;
    }

    std::cout << "\nAtomic wait, notify_one" << std::endl; // c++ 20
    {
        std::atomic<int> dataReady{0};

        auto worker = [&dataReady]()
        {
            std::cout << "Worker: Waiting for data...\n";
            dataReady.wait(0); // Wait until dataReady is not 0
            std::cout << "Worker: Data is ready! Processing...\n";
        };

        auto signal = [&dataReady]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate some work
            dataReady.store(1);
            std::cout << "Signal: Data is ready. Notifying worker...\n";
            dataReady.notify_one(); // Notify the waiting worker
        };

        std::thread workerThread(worker);
        std::thread signalThread(signal);

        workerThread.join();
        signalThread.join();
    }

    std::cout << "\nAtomic wait, notify_all" << std::endl; // c++ 20
    {
        std::atomic<int> dataReady{0};

        auto worker = [&dataReady]()
        {
            std::cout << "Worker: Waiting for data...\n";
            dataReady.wait(0); // Wait until dataReady is not 0
            std::cout << "Worker: Data is ready! Processing...\n";
        };

        auto signal = [&dataReady]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate some work
            dataReady.store(1);
            std::cout << "Signal: Data is ready. Notifying worker...\n";
            dataReady.notify_all(); // Notify the waiting worker
        };

        std::thread workerThread(worker);
        std::thread workerThread1(worker);
        std::thread signalThread(signal);

        workerThread.join();
        workerThread1.join();
        signalThread.join();
    }

    std::cout << "\nAtomic_flag wait, notify_one" << std::endl; // c++ 20
    {
        // std::atomic_flag dataReady{false}; // it is not standarted
        std::atomic_flag dataReady = ATOMIC_FLAG_INIT;

        auto worker = [&dataReady]()
        {
            std::cout << "Flag Worker: Waiting for data...\n";
            dataReady.wait(false); // Wait until dataReady is not false
            std::cout << "Flag Worker: Data is ready! Processing...\n";
        };

        auto signal = [&dataReady]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate some work
            dataReady.test_and_set();
            std::cout << "Flag Signal: Data is ready. Notifying worker...\n";
            dataReady.notify_one(); // Notify the waiting worker
        };

        std::thread workerThread(worker);
        std::thread signalThread(signal);

        workerThread.join();
        signalThread.join();
    }

    std::cout << "\nAtomic_flag wait, notify_all" << std::endl; // c++ 20
    {
        // std::atomic_flag dataReady{1}; // it is not standarted
        std::atomic_flag dataReady = ATOMIC_FLAG_INIT;
        dataReady.test_and_set();

        auto worker = [&dataReady]()
        {
            std::cout << "Flag Worker: Waiting for data...\n";
            atomic_flag_wait(&dataReady, true); // Wait until dataReady is not true
            std::cout << "Flag Worker: Data is ready! Processing...\n";
        };

        auto signal = [&dataReady]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate some work
            dataReady.clear();
            std::cout << "Flag Signal: Data is ready. Notifying worker...\n";
            atomic_flag_notify_all(&dataReady); // Notify the waiting workers
        };

        std::thread workerThread(worker);
        std::thread workerThread1(worker);
        std::thread signalThread(signal);

        workerThread.join();
        workerThread1.join();
        signalThread.join();
    }
    std::cout << std::endl
              << "Atomic end" << std::endl;

    return 0;
}