#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <thread>
#include <algorithm>
#include <memory>
#include <exception>

/*
mutex: lock(), unlock(), try_lock()

timed_mutex: lock(), unlock(), try_lock(), try_lock_for(), try_lock_until()

recursive_mutex:

recursive_timed_mutex:

shared_mutex: lock(), unlock(), try_lock(), lock_shared(), unlock_shared(), try_lock_shared()

shared_timed_mutex: lock(), unlock(), try_lock(), try_lock_until(), try_lock_for(),
                    lock_shared(), unlock_shared(), try_lock_shared(), try_lock_shared_until(), try_lock_shared_for()

*/

/*
lock_guard (C++11): RAII wrapper for a single mutex

scoped_lock (C++11): deadlock-avoiding RAII wrapper for multiple mutexes
                     if 0 mutex - does nothing, if 1 mutex - mutex.lock(), if > 1 mutex - std::lock()

unique_lock (C++11): implements movable mutex ownership wrapper with adopt, defer, try_lock_for, try_lock_until
shared_lock (C++14): implements movable mutex ownership wrapper for shared(timed) mutex. Same as unique_lock - only shared mutability
*/

/*
std::defer_lock  - must be no-locked mutex, can locked after
std::try_to_lock - uses try_lock()
std::adopt_lock  - must be locked mutex
*/

class X
{
    std::recursive_mutex m{};
    std::string shared{};

public:
    void fun1()
    {
        std::lock_guard<std::recursive_mutex> lk(m);
        shared = "fun1";
        std::cout << "in fun1, shared variable is now " << shared << '\n';
    }
    void fun2()
    {
        std::lock_guard<std::recursive_mutex> lk(m);
        shared = "fun2";
        std::cout << "in fun2, shared variable is now " << shared << '\n';
        fun1(); // recursive lock becomes useful here
        std::cout << "back in fun2, shared variable is " << shared << '\n';
    }
};

class ThreadSafeStringNoShared
{
public:
    ThreadSafeStringNoShared(const std::string &str) : str_(str), m() {}

    void Set(const std::string &str)
    {
        std::scoped_lock sl(m);
        str_ = str;
    }

    std::string Get()
    {
        std::scoped_lock sl(m);
        std::string res = str_;
        return res;
    }

private:
    std::string str_;
    mutable std::mutex m; // should be shared
    friend void swap(ThreadSafeStringNoShared &l, ThreadSafeStringNoShared &r);
};

void swap(ThreadSafeStringNoShared &l, ThreadSafeStringNoShared &r)
{
    std::cout << "ThreadSafeStringNoShared" << std::endl;
    std::scoped_lock sl(l.m, r.m); // calls std::lock(l.m, r.m)
    std::swap(l.str_, r.str_);
    // calls std::unlock(l.m, r.m)
}

class ThreadSafeString
{
public:
    ThreadSafeString(const std::string &str) : str_(str), m() {}

    void Set(const std::string &str)
    {
        std::scoped_lock sl(m);
        str_ = str;
    }

    std::string Get()
    {
        std::shared_lock sl(m);
        std::string res = str_;
        return res;
    }

private:
    std::string str_;
    mutable std::shared_mutex m;
};

class Mutex
{
public:
    Mutex(bool throw_ = false, bool throw_unlock_ = false) : throw_exception(throw_), throw_unlock(throw_unlock_) {}
    void lock()
    {
        std::cout << "Mutex lock" << std::endl;
        if (throw_exception)
        {
            throw std::runtime_error("My mutex exception from lock");
        }
    }
    bool try_lock()
    {
        std::cout << "Mutex try_lock" << std::endl;
        if (throw_exception)
        {
            throw std::runtime_error("My mutex exception from try_lock");
        }
        return true;
    }
    void unlock()
    {
        if (throw_unlock)
        {
            throw std::runtime_error("My mutex exception from unlock"); // abort program
        }

        std::cout << "Mutex lock" << std::endl;
    }

private:
    bool throw_exception;
    bool throw_unlock;
};

int main()
{
    std::cout << "Mutex start" << std::endl;

    std::cout << std::endl
              << "Mutex" << std::endl;
    {
        int count = 0;
        std::mutex m;

        auto worker = [&count, &m]()
        {
            m.lock();
            // m.try_lock(); // lock if can
            std::cout << "Worker start" << std::endl;
            ++count;
            std::cout << "Worker end. Count = " << count << std::endl;
            m.unlock();
            m.unlock(); // can unlock more than 1 time
        };

        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i)
        {
            threads.emplace_back(worker);
        }

        std::for_each(threads.begin(), threads.end(), [](std::thread &thr)
                      { thr.join(); });
    }
    /*
    std::try_lock and std::lock
    ----------------------------------------------------------------------------------------------------------------------------------------------------
    */

    std::cout << std::endl
              << "try_lock and lock" << std::endl;
    {
        std::mutex m1, m2, m3;
        auto res = std::try_lock(m3, m2, m1); // >= 2 mutexes must be
        if (res == -1)
        {
            std::cout << "std::try_lock Success" << std::endl;
            m1.unlock();
            m2.unlock();
            m3.unlock();
        }
        else
        {
            std::cout << "std::try_lock Failed" << std::endl;
        }

        std::cout << "std::lock exception" << std::endl;
        try
        {
            Mutex m(true, false), mm1(false, false);
            std::lock(m2, mm1, m, m3);
            std::cout << "std::lock is Success" << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cout << "std::lock: " << e.what() << std::endl;
        }
        std::cout << "std::try_lock exception" << std::endl;

        try
        {
            Mutex m(true, false), mm1(false, false);
            auto res = std::try_lock(m3, mm1, m, m1); // >= 2 mutexes must be
            if (res == -1)
            {
                std::cout << "std::try_lock Success" << std::endl;
                m1.unlock();
                m3.unlock();
                m.unlock();
                mm1.unlock();
            }
            else
            {
                std::cout << "std::try_lock Failed = " << res << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cout << "std::try_lock: " << e.what() << std::endl;
        }
    }
    /*
    lock_guard
    ----------------------------------------------------------------------------------------------------------------------------------------------------
    */
    std::cout << std::endl
              << "lock_guard" << std::endl;
    {
        int count = 0;
        std::mutex m;

        auto worker = [&count, &m]()
        {
            // std::lock_guard<std::mutex> lg(m);
            std::lock_guard lg(m); // calls m.lock
            std::cout << "Worker start" << std::endl;
            ++count;
            std::cout << "Worker end. Count = " << count << std::endl;

            // lg calls m.unlock
        };

        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i)
        {
            threads.emplace_back(worker);
        }

        std::for_each(threads.begin(), threads.end(), [](std::thread &thr)
                      { thr.join(); });
    }

    std::cout << std::endl
              << "lock_guard adopt_lock" << std::endl;
    {
        int count = 0;
        std::mutex m;
        auto job = [](std::mutex &m, int &count)
        {
            std::lock_guard<std::mutex> lg(m, std::adopt_lock);
            std::cout << "Worker start" << std::endl;
            ++count;
            std::cout << "Worker end. Count = " << count << std::endl;
        };

        auto worker = [&count, &m, &job]()
        {
            std::lock_guard<std::mutex> lg(m);
            // std::lock_guard<std::mutex> lg1 = std::move(lg); // nomoveble
            std::lock_guard<std::mutex> lg1(m, std::adopt_lock); // can unlock mutex more than 1 time
            job(m, count);
        };

        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i)
        {
            threads.emplace_back(worker);
        }

        std::for_each(threads.begin(), threads.end(), [](std::thread &thr)
                      { thr.join(); });
    }
    /*
    scoped_lock
    ----------------------------------------------------------------------------------------------------------------------------------------------------
    */
    std::cout << std::endl
              << "scoped_lock" << std::endl;
    {
        {
            [[maybe_unused]] std::scoped_lock sl; // does nothing
        }

        int count = 0;
        std::mutex m;

        auto worker = [&count, &m]()
        {
            std::lock_guard lg(m); // calls m.lock()
            std::cout << "Worker start" << std::endl;
            ++count;
            std::cout << "Worker end. Count = " << count << std::endl;
            // lg calls m.unlock()
        };

        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i)
        {
            threads.emplace_back(worker);
        }

        std::for_each(threads.begin(), threads.end(), [](std::thread &thr)
                      { thr.join(); });
    }

    std::cout << std::endl
              << "scoped_lock adopt_lock" << std::endl;
    {
        int count = 0;
        std::mutex m;

        auto job = [](std::mutex &m, int &count)
        {
            std::scoped_lock lg(std::adopt_lock, m);
            std::cout << "Worker start" << std::endl;
            ++count;
            std::cout << "Worker end. Count = " << count << std::endl;
        };

        auto worker = [&count, &m, &job]()
        {
            std::scoped_lock lg(m);
            // std::scoped_lock<std::mutex> lg1 = std::move(lg); // nomoveble
            std::scoped_lock lg1(std::adopt_lock, m); // can unlock mutex more than 1 time
            job(m, count);
        };

        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i)
        {
            threads.emplace_back(worker);
        }

        std::for_each(threads.begin(), threads.end(), [](std::thread &thr)
                      { thr.join(); });
    }
    std::cout << std::endl
              << "scoped_lock multiple mutexes" << std::endl;
    {
        ThreadSafeStringNoShared r("Str1");
        ThreadSafeStringNoShared l("Str2");
        std::cout << "r = " << r.Get() << ", l = " << l.Get() << std::endl;
        swap(r, l);
        std::cout << "r = " << r.Get() << ", l = " << l.Get() << std::endl;
    }

    /*
    unique_lock
    ----------------------------------------------------------------------------------------------------------------------------------------------------
    */
    std::cout << std::endl
              << "unique_lock" << std::endl;
    {
        {
            std::unique_lock<Mutex> m;
        }
        int count = 0;
        std::mutex m;

        {
            std::unique_lock<std::mutex> ul;     // does nothing
            std::unique_lock ul1(std::move(ul)); // movable
        }

        auto worker = [&count, &m]()
        {
            // std::unique_lock<std::mutex> ul(m);
            std::unique_lock ul(m); // calls m.lock
            std::cout << "Worker start" << std::endl;
            ++count;
            std::cout << "Worker end. Count = " << count << std::endl;

            // ul calls m.unlock
        };

        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i)
        {
            threads.emplace_back(worker);
        }

        std::for_each(threads.begin(), threads.end(), [](std::thread &thr)
                      { thr.join(); });
    }

    std::cout << std::endl
              << "unique_lock" << std::endl;
    {
        int count = 0;
        std::mutex m;

        {
            std::unique_lock<std::mutex> ul;     // does nothing
            std::unique_lock ul1(std::move(ul)); // movable
        }
        {
            std::unique_lock ul(m, std::defer_lock); // does not call m.lock
            std::cout << "ul owns_lock " << (ul.owns_lock() ? "owns mutex" : "does not own mutex") << std::endl;

            ul.lock();
            std::cout << "ul owns_lock after lock " << (ul.owns_lock() ? "owns mutex" : "does not own mutex") << std::endl;

            ul.unlock();
            std::cout << "ul owns_lock after unlock " << (ul.owns_lock() ? "owns mutex" : "does not own mutex") << std::endl;
            std::cout << "ul " << (ul.mutex() != nullptr ? "has mutex" : "does not have mutex") << std::endl;

            ul.lock();
            auto ass_mut = ul.mutex();
            std::cout << "ul mutex try lock:  " << (ass_mut->try_lock() ? "true, mutex was available" : "false, mutex was not available") << std::endl;
            std::cout << "ul mutex try lock:  " << (ass_mut->try_lock() ? "true, mutex was available" : "false, mutex was not available") << std::endl;

            std::unique_lock try_ul(*ass_mut, std::try_to_lock);
            std::cout << "try_ul owns_lock after unlock " << (try_ul.owns_lock() ? "owns mutex" : "does not own mutex") << std::endl;
            std::cout << "try_ul " << (try_ul.mutex() != nullptr ? "has mutex" : "does not have mutex") << std::endl;
            try_ul.release();

            std::unique_lock move_ul(std::move(ul));
            std::cout << "move_ul owns_lock after move constructor " << (move_ul ? "owns mutex" : "does not own mutex") << std::endl;
            std::cout << "ul " << (ul.mutex() != nullptr ? "has mutex" : "does not have mutex") << std::endl;
            std::cout << "ul owns_lock after move " << (ul ? "owns mutex" : "does not own mutex") << std::endl;

            ul = std::move(move_ul);
            std::cout << "ul owns_lock after move assisgning " << (ul ? "owns mutex" : "does not own mutex") << std::endl;

            auto mut = ul.release();
            std::cout << "ul owns_lock after release " << (ul ? "owns mutex" : "does not own mutex") << std::endl;

            mut->unlock();
        }
        auto worker = [&count, &m]()
        {
            std::unique_lock ul(m, std::defer_lock); // does not call m.lock
            ul.lock();
            std::cout << "Worker start" << std::endl;
            ++count;
            std::cout << "Worker end. Count = " << count << std::endl;
            // ul calls m.unlock
        };

        std::vector<std::thread> threads;
        for (int i = 0; i < 3; ++i)
        {
            threads.emplace_back(worker);
        }

        std::for_each(threads.begin(), threads.end(), [](std::thread &thr)
                      { thr.join(); });
    }
    std::cout << std::endl
              << "unique_lock lock for and lock until" << std::endl;

    {
        int count = 0;
        std::timed_mutex m;

        auto worker = [&count, &m]()
        {
            std::unique_lock ul(m);
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "Worker start" << std::endl;
            ++count;
            std::cout << "Worker end. Count = " << count << std::endl;
        };
        auto waiter = [&count, &m]()
        {
            // std::unique_lock ul(m, std::chrono::seconds(1));
            std::unique_lock ul(m, std::defer_lock);

            if (ul.try_lock_for(std::chrono::seconds(1)))
            {

                std::cout << "Worker start" << std::endl;
                ++count;
                std::cout << "Worker end. Count = " << count << std::endl;
            }
            else
            {
                std::cout << "waiter skips work" << std::endl;
            }
        };
        std::thread w1(worker);
        std::thread w2(waiter);
        w1.join();
        w2.join();
    }

    std::cout << std::endl
              << "shared_lock" << std::endl;
    {
        {
            std::shared_mutex sh_mutex;
            sh_mutex.lock_shared();
            sh_mutex.unlock_shared();
            // sh_mutex.unlock(); // undefine behavior
        }
        ThreadSafeString str("String1");

        auto setter = [&str]()
        {
            str.Set("Thread from the setter");
            std::cout << "String was set: " << str.Get() << std::endl;
        };

        auto getter = [&str]()
        {
            std::cout << "String = " << str.Get() << std::endl;
        };

        std::thread set(setter);
        std::thread get1(getter);
        std::thread get2(getter);

        set.join();
        get1.join();
        get2.join();
    }
    std::cout << std::endl
              << "recursive_mutex" << std::endl;
    {
        X x;
        std::thread t1(&X::fun1, &x);
        std::thread t2(&X::fun2, &x);
        t1.join();
        t2.join();
    }
    std::cout << "Mutex end" << std::endl;
}