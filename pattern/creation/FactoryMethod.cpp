/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Factory Method when you don’t know beforehand the exact types and
    dependencies of the objects your code should work with.

    Use the Factory Method when you want to provide users of your library or
    framework with a way to extend its internal components.

    Use the Factory Method when you want to save system resources by reusing
    existing objects instead of rebuilding them each time.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    You avoid tight coupling between the creator and the concrete products.

    Single Responsibility Principle.
    You can move the product creation code into one place in the program, making the code easier to support.

    Open/Closed Principle.
    You can introduce new types of products into the program without breaking existing client code.
Cons:
    The code may become more complicated since you need to introduce a lot of new subclasses to implement the pattern.
    The best case scenario is when you’re introducing the pattern into an existing hierarchy of creator classes.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <memory>

/*     Product         */
class IOnu
{
public:
    IOnu(const std::string mac) : mac_(mac) {}
    virtual ~IOnu() = default;

    const std::string &GetMac() const { return mac_; }

    virtual void Connect() = 0;

private:
    std::string mac_;
};

class EponOnu final : public IOnu
{
public:
    EponOnu(const std::string mac, int tag) : IOnu(mac), tag_(tag) {}

    void Connect() override
    {
        std::cout << "Connect to EPON Onu: mac = " << GetMac() << ", tag = " << tag_ << std::endl;
    }

private:
    int tag_;
};

class XgsOnu final : public IOnu
{
public:
    XgsOnu(const std::string mac, int port) : IOnu(mac), port_(port) {}

    void Connect() override
    {
        std::cout << "Connect to XGS Onu: mac = " << GetMac() << ", port = " << port_ << std::endl;
    }

private:
    int port_;
};
/*     Product end     */

/*     Creator         */
class ICreator
{
public:
    ICreator() = default;
    virtual ~ICreator() = default;
    virtual std::shared_ptr<IOnu> FactoryMethod(const std::string mac) = 0;
};

class EponCreator final : public ICreator
{
public:
    EponCreator(int tag) : tag_(tag) {}

    std::shared_ptr<IOnu> FactoryMethod(const std::string mac) override
    {
        std::cout << "Factory method of Epon: tag = " << tag_ << std::endl;

        return std::make_shared<EponOnu>(mac, tag_);
    }

private:
    int tag_;
};

class XgsCreator final : public ICreator
{
public:
    XgsCreator(int port) : port_(port) {}

    std::shared_ptr<IOnu> FactoryMethod(const std::string mac) override
    {
        std::cout << "Factory method of XGS: port = " << port_ << std::endl;

        return std::make_shared<XgsOnu>(mac, port_);
    }

private:
    int port_;
};
/*     Creator end     */

void App(std::shared_ptr<ICreator> creator)
{
    std::cout << "App start" << std::endl;

    std::shared_ptr<IOnu> onu1 = creator->FactoryMethod("MAC-1");
    onu1->Connect();

    std::shared_ptr<IOnu> onu2 = creator->FactoryMethod("MAC-2");
    onu2->Connect();

    std::cout << "App end" << std::endl;
}

int main()
{
    std::cout << "Factory Method 1" << std::endl
              << std::endl;

    std::shared_ptr<ICreator> creator;

    // if (Onus are EPON)
    creator = std::make_shared<EponCreator>(233 /* tag */);
    // else // XGS Onus
    //     creator = std::make_shared<XgsCreator>(233 /* tag */);

    App(creator);

    std::cout << std::endl
              << "Factory Method 2" << std::endl
              << std::endl;

    // if (Onus are EPON)
    // creator = std::make_shared<EponCreator>(233 /* tag */);
    // else // XGS Onus
    creator = std::make_shared<XgsCreator>(233 /* tag */);

    App(creator);

    return 0;
}