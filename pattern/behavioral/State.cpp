/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the State pattern when you have an object that behaves differently depending on its current state,
    the number of states is enormous, and the state-specific code changes frequently.

    Use the pattern when you have a class polluted with massive conditionals that
    alter how the class behaves according to the current values of the class’s field

    Use State when you have a lot of duplicate code across similar states and transitions of a condition-based state machine.


* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    Open/Closed Principle.
    Organize the code related to particular states into separate classes.

    Introduce new states without changing existing state classes or the context.

    Simplify the code of the context by eliminating bulky state machine conditionals.

Cons:
    Applying the pattern can be overkill if a state machine has only a few states or rarely changes.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <memory>
#include <ios>
#include <algorithm>
#include <functional>
#include <variant>
#include <exception>
#include <stdexcept>
#include <vector>
#include <numeric>
#include <thread>
#include <chrono>

class TrafficLightState
{
public:
    virtual ~TrafficLightState() = default;
    virtual void handleRequest() = 0;
    virtual std::string getState() const = 0;
};

class RedLightState : public TrafficLightState
{
public:
    void handleRequest() override
    {
        std::cout << "Red Light: Stop!" << std::endl;
    }

    std::string getState() const override
    {
        return "Red";
    }
};

class YellowLightState : public TrafficLightState
{
public:
    void handleRequest() override
    {
        std::cout << "Yellow Light: Slow down!" << std::endl;
    }

    std::string getState() const override
    {
        return "Yellow";
    }
};

class GreenLightState : public TrafficLightState
{
public:
    void handleRequest() override
    {
        std::cout << "Green Light: Go!" << std::endl;
    }

    std::string getState() const override
    {
        return "Green";
    }
};

class TrafficLight
{
public:
    TrafficLight(TrafficLightState *initialState) : currentState_(initialState) {}

    void setState(TrafficLightState *newState)
    {
        currentState_ = newState;
    }

    void request()
    {
        currentState_->handleRequest();
    }

    std::string getState() const
    {
        return currentState_->getState();
    }

private:
    TrafficLightState *currentState_;
};

int main()
{
    std::cout << std::endl
              << "State 1" << std::endl
              << std::endl;

    RedLightState redLight;
    YellowLightState yellowLight;
    GreenLightState greenLight;

    TrafficLight trafficLight(&redLight);

    std::cout << "Current State: " << trafficLight.getState() << std::endl;
    trafficLight.request();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    trafficLight.setState(&yellowLight);
    std::cout << "Current State: " << trafficLight.getState() << std::endl;
    trafficLight.request();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    trafficLight.setState(&greenLight);
    std::cout << "Current State: " << trafficLight.getState() << std::endl;
    trafficLight.request();

    std::cout
        << std::endl
        << "State End" << std::endl
        << std::endl;

    return 0;
}
