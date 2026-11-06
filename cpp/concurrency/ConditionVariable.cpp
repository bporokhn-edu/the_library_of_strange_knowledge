#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <stop_token>

std::mutex m;
std::condition_variable cv;
// std::condition_variable_any cv; // same as condition variable but can operate with different types of mutex(even custom mutex type)
// can wait for the stop_token(if stop_requested - run stuff)
std::string data;
bool ready = false;
bool processed = false;

/*
condition_variable::wait
    wait(unique_lock):
        wait for the notify, if notify was before the wait - race condition(threads wait for infinity)
    wait(unique_lock, predicate):
        mutex.lock();
        while(predicate != true) {
            mutex.unlock();
            wait for notifying or wake up(Spurious wakeup: weard bihavior)
            mutex.lock()
        }
        go out from wait only if predicate is true;
*/

void worker_thread()
{
    std::cout << "worker_thread: start" << std::endl;
    // wait until main() sends data
    std::unique_lock lk(m);
    std::cout << "worker_thread: mutex is locked" << std::endl;
    cv.wait(lk, []
            { std::cout << "Ready = " << ready << std::endl;
            return ready; });

    // after the wait, we own the lock
    std::cout << "Worker thread is processing data\n";
    data += " after processing";

    // send data back to main()
    processed = true;
    std::cout << "Worker thread signals data processing completed\n";

    // manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    lk.unlock();
    cv.notify_one();
}

std::condition_variable cv1;
std::mutex cv_m; // This mutex is used for three purposes:
                 // 1) to synchronize accesses to i
                 // 2) to synchronize accesses to std::cerr
                 // 3) for the condition variable cv
int i = 0;

void waits()
{
    std::unique_lock<std::mutex> lk(cv_m);
    std::cerr << "Waiting... \n";

    /*
    If any of the following conditions is satisfied, the behavior is undefined:
        1.lock.owns_lock() is false.
        2.lock.mutex() is not locked by the calling thread.
        3.If some other threads are also waiting on *this, lock.mutex() is different from the mutex unlocked
            by the waiting functions (wait, wait_for and wait_until) called on *this by those threads.
    */

    // cv1.wait(lk); // wait for the notify
    cv1.wait(lk, []
             { return i == 1; }); // waite for the i == 1, then for the notifying
    /*
    cv.wait:
     Atomically calls lock.unlock() and blocks on *this.
     The thread will be unblocked when notify_all() or notify_one() is executed. It may also be unblocked spuriously.
     When unblocked, calls lock.lock() (possibly blocking on the lock), then returns.
    */
    std::cerr << "...finished waiting. i == 1\n";
}

void waits_for()
{
    std::unique_lock<std::mutex> lk(cv_m);
    std::cerr << "Waiting for... \n";

    /*
    If any of the following conditions is satisfied, the behavior is undefined:
        1.lock.owns_lock() is false.
        2.lock.mutex() is not locked by the calling thread.
        3.If some other threads are also waiting on *this, lock.mutex() is different from the mutex unlocked
            by the waiting functions (wait, wait_for and wait_until) called on *this by those threads.
    */

    std::cv_status res = cv1.wait_for(lk, std::chrono::seconds(1));
    if (res == std::cv_status::no_timeout)
    {
        std::cerr << "no_timeout...finished waiting for. i == 1 \n";
    }
    else
    {
        std::cerr << "timeout...finished waiting for. i == " << i << "\n";
    }

    // bool res = cv1.wait_for(lk, std::chrono::seconds(1), []
    //                         { return i == 1; });
    // if (res)
    // {
    //     std::cerr << "no_timeout...finished waiting for. i == 1 \n";
    // }
    // else
    // {
    //     std::cerr << "timeout...finished waiting for. i == " << i << "\n";
    // }
}
void signals()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> lk(cv_m);
        std::cerr << "Notifying...\n";
    }
    cv1.notify_all(); // no affect - before waits

    std::this_thread::sleep_for(std::chrono::seconds(2));

    {
        std::lock_guard<std::mutex> lk(cv_m);
        i = 1;
        std::cerr << "Notifying again...\n";
    }
    // cv1.notify_one();
    cv1.notify_all();
}

class CustomMutex
{
public:
    void lock() { mtx.lock(); }
    void unlock() { mtx.unlock(); }
    bool try_lock() { return mtx.try_lock(); }

private:
    std::mutex mtx{};
};

CustomMutex customMtx;
std::condition_variable_any cv_any;
bool ready_any = false;
std::stop_source stopSource;

void worker_any(std::stop_token stoken)
{
    std::unique_lock<CustomMutex> lock(customMtx);

    std::cout << "worker_any waiting... stop requested = " << stoken.stop_requested() << std::endl;
    // Wait for the condition to be true or stop token to be requested
    cv_any.wait(lock, stoken, []
                { return ready_any; });
    std::cout << "worker_any notified stop requested = " << stoken.stop_requested() << std::endl;

    if (stoken.stop_requested())
    {
        std::cout << "Worker thread stopped\n";
    }
    else
    {
        std::cout << "Worker thread proceeding\n";
    }
}

std::condition_variable cv_exit;

bool ready_exit = false;
std::string result_exit; // some arbitrary type

void thread_func()
{
    std::cout << "thread_func\n";
    thread_local std::string thread_local_data = "42";

    std::unique_lock<std::mutex> lk(m);

    // assign a value to result using thread_local data
    result_exit = thread_local_data;
    ready_exit = true;

    std::cout << "thread_func notifuing on thread exit\n";
    std::notify_all_at_thread_exit(cv, std::move(lk));

} // 1. destroy thread_locals;
  // 2. unlock mutex;
  // 3. notify cv.

int main()
{
    std::cout << "notify one" << std::endl;

    std::thread worker(worker_thread);
    std::this_thread::sleep_for(std::chrono::seconds(2)); // wait for running of the worker

    std::cout << "main() start\n";
    data = "Example data";
    // send data to the worker thread
    {
        // cv.notify_one(); // can also work: do not use this way
        std::lock_guard lk(m);
        ready = true;
        std::cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();

    // wait for the worker
    {
        std::unique_lock lk(m);
        cv.wait(lk, []
                { return processed; });
    }
    std::cout << "Back in main(), data = " << data << '\n';

    worker.join();

    std::cout << std::endl
              << "notify all" << std::endl;
    {
        std::thread t1(signals);
        std::thread t2(waits), t3(waits), t4(waits), t5(waits_for);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
    }

    std::cout << std::endl
              << "condition variable any with stop token" << std::endl;
    {
        std::stop_token stopToken = stopSource.get_token();
        std::thread t(worker_any, stopToken);

        std::this_thread::sleep_for(std::chrono::seconds(1));

        {
            std::unique_lock<CustomMutex> lock(customMtx);
            ready = true;
            std::cout << "main() notifying all" << std::endl;
            cv_any.notify_all();
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "main() requesting stop" << std::endl;
        // Request stop
        stopSource.request_stop();

        t.join();
    }
    std::cout << "condition_variable_any end" << std::endl;
    std::cout << std::endl
              << "std::notify_all_at_thread_exit" << std::endl;
    {
        std::thread t(thread_func);
        t.detach();
        std::cout << "main() start" << std::endl;

        // do other work
        // ...

        // wait for the detached thread
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, []
                { return ready_exit; });

        // result is ready and thread_local destructors have finished, no UB
        std::cout << "result_exit = " << result_exit << std::endl;
    }
    std::cout << std::endl
              << "std::notify_all_at_thread_exit end" << std::endl;
}