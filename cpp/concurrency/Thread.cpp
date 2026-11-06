#include <iostream>
#include <fstream>
#include <thread>
#include <exception>
#include <chrono>
#include <mutex>
#include <pthread.h>
#include <cstring>
#include <atomic>

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

std::mutex iomutex;

int main()
{
    std::cout << "Thread start" << std::endl;
    std::cout << "Thread hardware concurrency = " << std::thread::hardware_concurrency() << std::endl;
    std::cout << "Main thread id = " << std::this_thread::get_id() << std::endl;

    std::cout << "Empty constructor" << std::endl;
    {
        std::thread t;
        std::thread t1 = std::move(t); // only move constructor
    }

    std::cout << "Lambda" << std::endl;
    {
        int param = 10;
        std::thread t([&param /* can be the reference */]
                      { std::cout << "Thread lambda: " << std::this_thread::get_id() << " param: " << param << std::endl; 
                      param +=10; });
        t.join();
        std::cout << "Thread main: " << std::this_thread::get_id() << " param: " << param << std::endl;
    }

    std::cout << "Function" << std::endl;
    {
        std::thread t(func, 11);
        t.join();
        int param = 12;
        std::thread tRef(funcRef, std::ref(param));
        tRef.join();
        std::cout << "Thread main: " << std::this_thread::get_id() << " param: " << param << std::endl;
    }

    std::cout << "Class" << std::endl;
    {
        Run run(14);
        std::thread t(&Run::func, run, 100);
        t.join();

        run.SetParam(16);
        std::thread tFunc(run);
        tFunc.join();
    }

    std::cout << "native_handle" << std::endl;
    {
        auto print_priority = [](int num)
        {
            // std::this_thread::sleep_for(std::chrono::seconds(1));
            using std::chrono::operator""s;
            auto awake_time =  std::chrono::steady_clock::now() + 1s;
            std::this_thread::sleep_until(awake_time);

            sched_param sch;
            int policy;
            pthread_getschedparam(pthread_self(), &policy, &sch);
            std::lock_guard<std::mutex> lk(iomutex);
            std::cout << "Thread " << num << " is executing at priority "
                      << sch.sched_priority << '\n';
        };

        std::thread t1(print_priority, 1), t2(print_priority, 2);

        sched_param sch;
        int policy;
        pthread_getschedparam(t1.native_handle(), &policy, &sch);
        sch.sched_priority = 20;
        if (pthread_setschedparam(t1.native_handle(), SCHED_FIFO, &sch))
            std::cout << "Failed to setschedparam: " << std::strerror(errno) << '\n';

        t1.join();
        t2.join();
    }

    std::cout << "Yield" << std::endl;
    {
        std::atomic<bool> ready{false};

        auto worker = [&ready]()
        {
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::seconds(1));
            ready = true;
            std::cout << "Worker: Work done, ready = true" << std::endl;
        };
        std::thread t(worker);

        // Busy-wait loop with yield
        while (!ready)
        {
            std::this_thread::yield(); //  typically, the calling thread will be moved to the end of the scheduling queue for its priority level.
        }

        std::cout << "Main: Detected ready = true" << std::endl;

        t.join();
    }

    std::cout << "Detach" << std::endl;
    {
        int param = 100;
        std::thread t([param /* can be the reference */]
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