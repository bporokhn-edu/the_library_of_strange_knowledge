/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Bridge pattern when you want to divide and organize a monolithic class that has several variants of some functionality
    (for example, if the class can work with various database servers).

    Use the pattern when you need to extend a class in several orthogonal (independent) dimensions.

    Use the Bridge if you need to be able to switch implementations at runtime.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    You can create platform-independent classes and apps.

    The client code works with high-level abstractions. It isn’t exposed to the platform details.

    Open/Closed Principle.
    You can introduce new abstractions and implementations independently from each other.

    Single Responsibility Principle.
    You can focus on high-level logic in the abstraction and on platform details in the implementation.

Cons:
    You might make the code more complicated by applying the pattern to a highly cohesive class.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <vector>
#include <memory>

class IOnu
{
public:
    virtual ~IOnu() = default;
    virtual void Connect() = 0;
    virtual void GetHwInfo() = 0;
    virtual void Configure() = 0;
    virtual void EnableNetwork() = 0;
};

class EponOnu : public IOnu
{
public:
    void Connect() override
    {
        std::cout << "EponOnu: Connect" << std::endl;
    }
    void GetHwInfo() override
    {
        std::cout << "EponOnu: GetHwInfo" << std::endl;
    }
    void Configure() override
    {
        std::cout << "EponOnu: Configure" << std::endl;
    }
    void EnableNetwork() override
    {
        std::cout << "EponOnu: EnableNetwork" << std::endl;
    }
};

class XgsOnu : public IOnu
{
public:
    void Connect() override
    {
        std::cout << "XgsOnu: Connect" << std::endl;
    }
    void GetHwInfo() override
    {
        std::cout << "XgsOnu: GetHwInfo" << std::endl;
    }
    void Configure() override
    {
        std::cout << "XgsOnu: Configure" << std::endl;
    }
    void EnableNetwork() override
    {
        std::cout << "XgsOnu: EnableNetwork" << std::endl;
    }
};

class Olt
{
public:
    Olt(const std::vector<std::shared_ptr<IOnu>> &onus = {}) : onus_{onus} {}
    void DiscoverOnus(const std::vector<std::shared_ptr<IOnu>> &onus)
    {
        for (const auto &onu : onus)
        {
            onus_.push_back(onu);
        }
    }

    void ConnectAll()
    {
        for (auto onu : onus_)
        {
            onu->GetHwInfo();
            onu->Connect();
        }
    }

    void EnableAll()
    {
        for (auto onu : onus_)
        {
            onu->Configure();
            onu->EnableNetwork();
        }
    }

private:
    // not good example: IOnu should be unique(not a vector);
    std::vector<std::shared_ptr<IOnu>> onus_;
};

int main()
{
    std::cout << "Bridge 1" << std::endl
              << std::endl;

    Olt olt;
    olt.DiscoverOnus({std::make_shared<EponOnu>(), std::make_shared<EponOnu>()});
    olt.ConnectAll();
    olt.EnableAll();

    std::cout << std::endl
              << "Bridge 2" << std::endl
              << std::endl;

    olt.DiscoverOnus({std::make_shared<XgsOnu>(), std::make_shared<XgsOnu>()});
    olt.ConnectAll();
    olt.EnableAll();

    return 0;
}