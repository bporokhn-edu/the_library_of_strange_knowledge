#include <future>
#include <iostream>
#include <thread>
#include <cmath>
#include <vector>
#include <functional>
#include <numeric>

/*
std::promise:
    pass to the function agrs, function will be run in the another thread

std::package_task:
    pack function(lambda, function, functor), get future, exec and get res from the future

std::async:
    async:	  the task is executed on a different thread, potentially by creating and launching it first
    deferred: task is executed on the calling thread the first time its result is requested (lazy evaluation)
*/

void sum(std::vector<int>::iterator first,
         std::vector<int>::iterator last,
         std::promise<int> accumulate_promise)
{
    int sum = std::accumulate(first, last, 0);
    accumulate_promise.set_value(sum); // Notify future
}

void do_work(std::promise<void> barrier)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    barrier.set_value();
}

// unique function to avoid disambiguating the std::pow overload set
int f(int x, int y) { return std::pow(x, y); }

void task_lambda()
{
    std::packaged_task<int(int, int)> task([](int a, int b)
                                           { return std::pow(a, b); });
    std::future<int> result = task.get_future();

    task(2, 9);

    std::cout << "task_lambda:\t" << result.get() << '\n';
}

void task_bind()
{
    std::packaged_task<int()> task(std::bind(f, 2, 11));
    std::future<int> result = task.get_future();

    task();

    std::cout << "task_bind:\t" << result.get() << '\n';
}

void task_thread()
{
    std::packaged_task<int(int, int)> task(f);
    std::future<int> result = task.get_future();

    std::thread task_td(std::move(task), 2, 10);
    task_td.join();

    std::cout << "task_thread:\t" << result.get() << '\n';
}

std::mutex m;

struct X
{
    void foo(int i, const std::string &str)
    {
        std::lock_guard<std::mutex> lk(m);
        std::cout << str << ' ' << i << '\n';
    }

    void bar(const std::string &str)
    {
        std::lock_guard<std::mutex> lk(m);
        std::cout << str << '\n';
    }

    int operator()(int i)
    {
        std::lock_guard<std::mutex> lk(m);
        std::cout << i << '\n';
        return i + 10;
    }
};

template <typename RandomIt>
int parallel_sum(RandomIt beg, RandomIt end)
{
    auto len = end - beg;
    if (len < 1000)
        return std::accumulate(beg, end, 0);

    RandomIt mid = beg + len / 2;
    auto handle = std::async(std::launch::async,
                             parallel_sum<RandomIt>, mid, end);
    int sum = parallel_sum(beg, mid);
    return sum + handle.get();
}

void taskWithException(std::promise<int> &prom)
{
    try
    {
        // Simulate an error
        throw std::runtime_error("Error in thread");
    }
    catch (...)
    {
        prom.set_exception(std::current_exception());
    }
}

int taskWithExceptionPt()
{
    throw std::runtime_error("Error in task");
    return 42; // Will not reach here
}

int main()
{
    std::cout << "Promise" << std::endl;
    {
        // Demonstrate using promise<int> to transmit a result between threads.
        std::vector<int> numbers = {1, 2, 3, 4, 5, 6};
        std::promise<int> accumulate_promise;
        std::future<int> accumulate_future = accumulate_promise.get_future();
        std::thread work_thread(sum, numbers.begin(), numbers.end(),
                                std::move(accumulate_promise));

        // future::get() will wait until the future has a valid result and retrieves it.
        // Calling wait() before get() is not needed
        // accumulate_future.wait(); // wait for result
        std::cout << "result=" << accumulate_future.get() << '\n';
        work_thread.join(); // wait for thread completion

        // Demonstrate using promise<void> to signal state between threads.
        std::promise<void> barrier;
        std::future<void> barrier_future = barrier.get_future();
        std::thread new_work_thread(do_work, std::move(barrier));
        barrier_future.wait();
        std::cout << "main thread is notified" << std::endl;
        new_work_thread.join();
    }
    std::cout << "Promise with the exception" << std::endl;
    {
        std::promise<int> prom;
        std::future<int> fut = prom.get_future();

        std::thread t(taskWithException, std::ref(prom));
        t.join();

        try
        {
            [[maybe_unused]] int result = fut.get(); // This will throw the exception
        }
        catch (const std::exception &e)
        {
            std::cout << "Caught exception: " << e.what() << std::endl;
        }
    }

    std::cout << std::endl
              << "Package task" << std::endl;
    {
        task_lambda();
        task_bind();
        task_thread();
    }
    std::cout << std::endl
              << "Package task with the exception" << std::endl;
    {
        std::packaged_task<int()> task(taskWithExceptionPt);
        std::future<int> fut = task.get_future();

        std::thread t(std::move(task));
        t.join();

        try
        {
            [[maybe_unused]] int result = fut.get(); // This will throw the exception
        }
        catch (const std::exception &e)
        {
            std::cout << "Caught exception: " << e.what() << std::endl;
        }
    }
    std::cout << std::endl
              << "shared_future" << std::endl;
    {
        std::promise<void> ready_promise, t1_ready_promise, t2_ready_promise;
        std::shared_future<void> ready_future(ready_promise.get_future());

        std::chrono::time_point<std::chrono::high_resolution_clock> start;

        auto fun1 = [&, ready_future]() -> std::chrono::duration<double, std::milli>
        {
            t1_ready_promise.set_value();
            ready_future.wait(); // waits for the signal from main()
            return std::chrono::high_resolution_clock::now() - start;
        };

        auto fun2 = [&, ready_future]() -> std::chrono::duration<double, std::milli>
        {
            t2_ready_promise.set_value();
            ready_future.wait(); // waits for the signal from main()
            return std::chrono::high_resolution_clock::now() - start;
        };

        auto fut1 = t1_ready_promise.get_future();
        auto fut2 = t2_ready_promise.get_future();

        auto result1 = std::async(std::launch::async, fun1);
        auto result2 = std::async(std::launch::async, fun2);

        // wait for the threads to become ready
        fut1.wait();
        fut2.wait();

        // the threads are ready, start the clock
        start = std::chrono::high_resolution_clock::now();

        // signal the threads to go
        ready_promise.set_value();

        std::cout << "Thread 1 received the signal "
                  << result1.get().count() << " ms after start\n"
                  << "Thread 2 received the signal "
                  << result2.get().count() << " ms after start\n";
    }
    std::cout << std::endl
              << "async" << std::endl;
    {
        std::vector<int> v(10000, 1);
        std::cout << "The sum is " << parallel_sum(v.begin(), v.end()) << '\n';

        X x;
        // Calls (&x)->foo(42, "Hello") with default policy:
        // may print "Hello 42" concurrently or defer execution
        auto a1 = std::async(&X::foo, &x, 42, "Hello");
        // Calls x.bar("world!") with deferred policy
        // prints "world!" when a2.get() or a2.wait() is called
        auto a2 = std::async(std::launch::deferred, &X::bar, x, "world!");
        // Calls X()(43); with async policy
        // prints "43" concurrently
        auto a3 = std::async(std::launch::async, X(), 43);
        a2.wait();                     // prints "world!"
        std::cout << a3.get() << '\n'; // prints "53"
    }
}