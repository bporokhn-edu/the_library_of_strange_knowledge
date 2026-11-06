/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Singleton pattern when a class in your program should have just a single instance available to all clients;
    for example, a single database object shared by different parts of the program.

    Use the Singleton pattern when you need stricter control over global variables.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    You can be sure that a class has only a single instance.

    You gain a global access point to that instance.

    The singleton object is initialized only when it’s requested for the first time.
Cons:
    Violates the Single Responsibility Principle. The pattern solves two problems at the time.

    The Singleton pattern can mask bad design, for instance, when the components of the program know too much about each other.

    The pattern requires special treatment in a multithreaded environment
    so that multiple threads won’t create a singleton object several times.

    It may be difficult to unit test the client code of the Singleton because many test frameworks rely on inheritance
    when producing mock objects. Since the constructor of the singleton class is private and overriding static methods
    is impossible in most languages, you will need to think of a creative way to mock the singleton.
    Or just don’t write the tests. Or don’t use the Singleton pattern.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <memory>

class Singelton {
public:
    static Singelton& GetInstance() {
        std::cout << "Get instance" << std::endl;
        static Singelton instance;
        return instance;
    }

    void Execute(const std::string& command) {
        std::cout << "Execute: " << command << std::endl;
    }

private:
    Singelton() = default;
    Singelton(const Singelton& singelton) = delete;
    const Singelton& operator=(const Singelton& singelton) = delete;
    Singelton(const Singelton&& singelton) = delete;
    const Singelton& operator=(const Singelton&& singelton) = delete;
};

int main()
{
    std::cout << "Singelton 1" << std::endl
              << std::endl;

    auto& singelton = Singelton::GetInstance();
    singelton.Execute("My Command");

    std::cout << std::endl
              << "Singelton 2" << std::endl
              << std::endl;

    auto& singelton1 = Singelton::GetInstance();
    singelton1.Execute("My Command 2");

    return 0;
}