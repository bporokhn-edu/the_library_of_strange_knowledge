#include <iostream>
#include <fstream>
#include <exception>
#include <chrono>
#include <mutex>
#include <pthread.h>
#include <cstring>
#include <thread>
#include <condition_variable>
#include <sstream>

/*
Difference between thread and jthread:
jthread call join() in destructor if it is joinable()

Unlike std::thread, the jthread logically holds an internal private member of type std::stop_source,
which maintains a shared stop-state. The jthread constructor accepts a function that takes a
std::stop_token as its first argument, which will be passed in by the jthread from its internal std::stop_source.
This allows the function to check if stop has been requested during its execution, and return if it has.
*/

void func(int param)
{
    std::cout << "Thread func: " << std::this_thread::get_id() << " params: " << param << std::endl;

    // throw std::bad_alloc(); // !terminate! called after throwing an instance of 'std::bad_alloc'
}

void funcRef(int &param)
{
    std::cout << "Thread funcRef: " << std::this_thread::get_id() << " params: " << param << std::endl;
    param += 10;
}

class Run
{
public:
    Run(int param) : param_(param) {}

    void SetParam(int param)
    {
        param_ = param;
    }

    void func(int param)
    {
        std::cout << "Thread Run::func: " << std::this_thread::get_id() << " param: " << param_ << ", " << param << std::endl;
    }

    void operator()()
    {
        std::cout << "Thread Run::operator(): " << std::this_thread::get_id() << " param: " << param_ << std::endl;
    }

private:
    int param_;
};

// Use a helper class for atomic std::cout streaming.
class Writer
{
    std::ostringstream buffer{};

public:
    ~Writer()
    {
        std::cout << buffer.str();
    }
    Writer &operator<<(auto input)
    {
        buffer << input;
        return *this;
    }
};

std::mutex iomutex;

int main()
{
    std::cout << "Thread start" << std::endl;
    std::cout << "Thread hardware concurrency = " << std::thread::hardware_concurrency() << std::endl;
    std::cout << "Main thread id = " << std::this_thread::get_id() << std::endl;

    std::cout << "Empty constructor" << std::endl;
    {
        std::jthread t;
        std::jthread t1 = std::move(t); // only move constructor
    }

    std::cout << "Lambda" << std::endl;
    {
        int param = 10;
        {
            std::jthread t([&param /* can be the reference */]
                           { std::cout << "Thread lambda: " << std::this_thread::get_id() << " param: " << param << std::endl; 
                      param +=10; });
        }
        std::cout << "Thread main: " << std::this_thread::get_id() << " param: " << param << std::endl;
    }

    std::cout << "Function" << std::endl;
    {
        {
            std::jthread t(func, 11);
        }
        int param = 12;
        {
            std::jthread tRef(funcRef, std::ref(param));
        }
        std::cout << "Thread main: " << std::this_thread::get_id() << " param: " << param << std::endl;
    }

    std::cout << "Class" << std::endl;
    {
        Run run(14);
        {
            std::jthread t(&Run::func, run, 100);
        }

        run.SetParam(16);
        {
            std::jthread tFunc(run);
        }
    }

    std::cout << "native_handle" << std::endl;
    {
        auto print_priority = [](int num)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            sched_param sch;
            int policy;
            pthread_getschedparam(pthread_self(), &policy, &sch);
            std::lock_guard<std::mutex> lk(iomutex);
            std::cout << "Thread " << num << " is executing at priority "
                      << sch.sched_priority << '\n';
        };

        {
            std::jthread t1(print_priority, 1), t2(print_priority, 2);

            sched_param sch;
            int policy;
            pthread_getschedparam(t1.native_handle(), &policy, &sch);
            sch.sched_priority = 20;
            if (pthread_setschedparam(t1.native_handle(), SCHED_FIFO, &sch))
                std::cout << "Failed to setschedparam: " << std::strerror(errno) << '\n';
        }
    }

    using namespace std::literals::chrono_literals;
    std::cout << "std::stop_token" << std::endl;
    {
        auto f = [](std::stop_token stop_token, int value)
        {
            while (!stop_token.stop_requested())
            {
                std::cout << value++ << ' ' << std::flush;
                std::this_thread::sleep_for(200ms);
            }
            std::cout << std::endl;
        };

        std::jthread thread(f, 5); // prints 5 6 7 8 9 10 11 12 13 14 ... for approximately 3 seconds
        std::this_thread::sleep_for(2s);
        // The destructor of jthread calls request_stop() and join().
    }

    std::cout << std::endl
              << "std::stop_token 2" << std::endl;
    {
        std::jthread sleepy_worker(
            [](std::stop_token stoken)
            {
                for (int i = 10; i; --i)
                {
                    std::this_thread::sleep_for(300ms);
                    if (stoken.stop_requested())
                    {
                        std::cout << "Sleepy worker is requested to stop\n";
                        return;
                    }
                    std::cout << "Sleepy worker goes back to sleep\n";
                }
            });

        std::stop_token stop_token = sleepy_worker.get_stop_token();
        // A waiting worker thread
        // The condition variable will be awoken by the stop request.
        std::jthread waiting_worker(
            [](std::stop_token stoken)
            {
                std::mutex mutex;
                std::unique_lock lock(mutex);
                std::condition_variable_any().wait(lock, stoken, []
                                                   { return false; });
                std::cout << "Waiting worker is requested to stop\n";
                return;
            },
            stop_token);

        std::this_thread::sleep_for(2s);
        // std::jthread::request_stop() can be called explicitly:
        std::cout << "Requesting stop of sleepy worker\n";
        sleepy_worker.request_stop();
        sleepy_worker.join();
        std::this_thread::sleep_for(2s);
        std::cout << "Sleepy worker joined\n";
    }

    std::cout << std::endl
              << "std::stop_source" << std::endl;
    {

        auto print = [](std::string_view name, const std::stop_token &token)
        {
            std::cout << name << ": stop_possible = " << token.stop_possible();
            std::cout << ", stop_requested = " << token.stop_requested() << '\n';
        };

        auto finite_sleepy = [](std::stop_token stoken)
        {
            for (int i = 10; i; --i)
            {
                std::this_thread::sleep_for(300ms);
                if (stoken.stop_requested())
                {
                    std::cout << "  Sleepy worker is requested to stop\n";
                    return;
                }

                std::cout << "  Sleepy worker goes back to sleep\n";
            }
        };

        auto infinite_sleepy = []()
        {
            for (int i = 5; i; --i)
            {
                std::this_thread::sleep_for(300ms);
                std::cout << "  Run as long as we want\n";
            }
        };

        std::cout << std::boolalpha;

        // A worker thread that will listen to stop requests
        std::jthread stop_worker(finite_sleepy);

        // A worker thread that will only stop when completed
        std::jthread inf_worker(infinite_sleepy);

        std::stop_token def_token;
        std::stop_token stop_token = stop_worker.get_stop_token();
        std::stop_token inf_token = inf_worker.get_stop_token();
        print("def_token ", def_token);
        print("stop_token", stop_token);
        print("inf_token ", inf_token);

        std::cout << "\nRequest and join stop_worker:\n";
        stop_worker.request_stop();
        stop_worker.join();

        std::cout << "\nRequest and join inf_worker:\n";
        inf_worker.request_stop();
        inf_worker.join();
        std::cout << '\n';

        print("def_token ", def_token);
        print("stop_token", stop_token);
        print("inf_token ", inf_token);
    }

    std::cout << std::endl
              << "stop_callback" << std::endl;
    {
        // A worker thread.
        // It will wait until it is requested to stop.
        Writer() << "main thread's id: " << std::this_thread::get_id() << '\n';
        std::jthread worker([](std::stop_token stoken)
                            {
        Writer() << "Worker thread's id: " << std::this_thread::get_id() << '\n';
        // some work
        std::mutex mutex;
        std::unique_lock lock(mutex);
        std::condition_variable_any().wait(lock, stoken,
            [&stoken] { return stoken.stop_requested(); }); });

        // Register a stop callback on the worker thread.
        std::stop_callback callback(worker.get_stop_token(), []
                                    { Writer() << "Stop callback executed by thread: "
                                               << std::this_thread::get_id() << '\n'; });

        // Stop_callback objects can be destroyed prematurely to prevent execution.
        {
            std::stop_callback scoped_callback(worker.get_stop_token(), []
                                               {
            // This will not be executed.
            Writer() << "Scoped stop callback executed by thread: "
                << std::this_thread::get_id() << '\n'; });
        }

        // Demonstrate which thread executes the stop_callback and when.
        // Define a stopper function.
        auto stopper_func = [&worker]
        {
            if (worker.request_stop())
                Writer() << "Stop request executed by thread: "
                         << std::this_thread::get_id() << '\n';
            else
                Writer() << "Stop request not executed by thread: "
                         << std::this_thread::get_id() << '\n';
        };

        // Let multiple threads compete for stopping the worker thread.
        std::jthread stopper1(stopper_func);
        std::jthread stopper2(stopper_func);
        stopper1.join();
        stopper2.join();

        // After a stop has already been requested,
        // a new stop_callback executes immediately.
        Writer() << "Main thread: " << std::this_thread::get_id() << '\n';
        std::stop_callback callback_after_stop(worker.get_stop_token(), []
                                               { Writer() << "Stop callback executed by thread: "
                                                          << std::this_thread::get_id() << '\n'; });
    }

    std::cout << "Detach" << std::endl;
    {
        int param = 100;
        std::jthread t([param /* can be the reference */]
                       { std::this_thread::sleep_for(std::chrono::seconds(3));
                      std::ofstream file("example.txt");
                      file << "Thread lambda: " << std::this_thread::get_id() << " params: " << param << std::endl;

                    //   std::cout << "Thread lambda: " << std::this_thread::get_id() << " params: " << param << std::endl;
                      /* we can't see this print */ });
        t.detach(); // this thread will be terminated when main thread will be finished
        // if thread remains without join or detach and program is finished - !terminate!
    }
    std::cout << "Thread end" << std::endl;

    return 0;
}