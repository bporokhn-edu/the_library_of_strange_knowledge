#include <chrono>
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>

/*
semaphore::aquire():
    Atomically decrements the internal counter by 1 if it is greater than ​0​;
    otherwise blocks until it is greater than ​0​ and can successfully decrement the internal counter.
swmophore::release(std::ptrdiff_t update = 1):
    Atomically increments the internal counter by the value of update.
    Any thread(s) waiting for the counter to be greater than ​0​, such as due to being blocked in acquire,
    will subsequently be unblocked.
*/

const int MAX_CONCURRENT_THREADS = 3;
std::counting_semaphore<MAX_CONCURRENT_THREADS> semaphore(MAX_CONCURRENT_THREADS);

void worker(int id)
{
    std::cout << "Thread " << id << " is starting..." << std::endl;
    using namespace std::literals;
    std::this_thread::sleep_for(1s);
    // Try to acquire the semaphore
    semaphore.acquire();

    // Critical section (only MAX_CONCURRENT_THREADS threads can be here simultaneously)
    std::cout << "Thread " << id << " is working..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Release the semaphore
    std::cout << "Thread " << id << " is done." << std::endl;
    semaphore.release(1);
}

// global binary semaphore instances
// object counts are set to zero
// objects are in non-signaled state
std::binary_semaphore
    smphSignalMainToThread{0},
    smphSignalThreadToMain{0};

void ThreadProc()
{
    std::cout << "[thread] start\n";

    // wait for a signal from the main proc
    // by attempting to decrement the semaphore
    smphSignalMainToThread.acquire();

    // this call blocks until the semaphore's count
    // is increased from the main proc

    std::cout << "[thread] Got the signal\n"; // response message

    // wait for 3 seconds to imitate some work
    // being done by the thread
    using namespace std::literals;
    std::this_thread::sleep_for(3s);

    std::cout << "[thread] Send the signal\n"; // message

    // signal the main proc back
    smphSignalThreadToMain.release();
    std::cout << "[thread] end\n";
}

int main()
{
    std::cout << "binary semaphore" << std::endl;
    {
        // create some worker thread
        std::thread thrWorker(ThreadProc);
        using namespace std::literals;
        std::this_thread::sleep_for(3s);
        std::cout << "[main] Send the signal\n"; // message

        // signal the worker thread to start working
        // by increasing the semaphore's count
        smphSignalMainToThread.release();

        std::cout << "[main] Wait for the signal\n"; // message
        // wait until the worker thread is done doing the work
        // by attempting to decrement the semaphore's count
        smphSignalThreadToMain.acquire();

        std::cout << "[main] Got the signal\n"; // response message
        thrWorker.join();
    }
    std::cout << "counting semaphore" << std::endl;
    {
        std::vector<std::thread> threads;
        // Create and start 10 worker threads
        for (int i = 0; i < 10; ++i)
        {
            threads.emplace_back(worker, i);
        }

        // Join all threads
        for (auto &t : threads)
        {
            t.join();
        }
    }
}
