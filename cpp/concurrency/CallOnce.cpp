#include <iostream>
#include <mutex>
#include <thread>

std::once_flag flag1, flag2;

void simple_do_once()
{
    std::call_once(flag1, []()
                   { std::cout << "Simple example: called once\n"; });
}

void may_throw_function(bool do_throw, int num)
{
    std::cout << "may_throw_function, num = " << num << std::endl;
    if (do_throw)
    {
        std::cout << "Throw: call_once will retry\n"; // this may appear more than once
        throw std::runtime_error("may_throw_function " + std::to_string(num));
    }
    std::cout << "Did not throw, call_once will not attempt again\n"; // guaranteed once
}

void do_once(bool do_throw, int num)
{
    try
    {
        std::call_once(flag2, may_throw_function, do_throw, num);
    }
    catch (std::exception &e)
    {
        std::cout << "do_once exception: " << e.what() << std::endl;
    }
}

int main()
{
    std::cout << "Call once" << std::endl;
    std::thread st1(simple_do_once);
    std::thread st2(simple_do_once);
    std::thread st3(simple_do_once);
    std::thread st4(simple_do_once);
    st1.join();
    st2.join();
    st3.join();
    st4.join();
    std::cout << std::endl
              << "Call once with exception" << std::endl;

    std::thread t1(do_once, true, 1);
    std::thread t2(do_once, true, 2);
    std::thread t3(do_once, false, 3);
    std::thread t4(do_once, true, 4);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}