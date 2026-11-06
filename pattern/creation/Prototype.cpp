/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Prototype pattern when your code shouldn’t depend on the concrete classes of objects that you need to copy.

    Use the pattern when you want to reduce the number of subclasses that only differ in the way they initialize their respective object

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    You can clone objects without coupling to their concrete classes.

    You can get rid of repeated initialization code in favor of cloning pre-built prototypes.

    You can produce complex objects more conveniently.

    You get an alternative to inheritance when dealing with configuration presets for complex objects.
Cons:
    Cloning complex objects that have circular references might be very tricky.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <memory>
#include <unordered_map>

enum OnuPrototype
{
    EPON = 0,
    XGS = 1
};

class Prototype
{
public:
    Prototype() : mode_(), tag_() { std::cout << "Prototype default constructor" << std::endl; }
    Prototype(std::string mode) : mode_(mode), tag_(0) { std::cout << "Prototype constructor" << std::endl; }

    virtual ~Prototype() = default;
    virtual std::shared_ptr<Prototype> Clone() const = 0;
    virtual void Start(int tag)
    {
        tag_ = tag;
        std::cout << "Start: " << mode_ << " on " << tag_ << " tag" << std::endl;
    }

protected:
    std::string mode_;
    int tag_;
};

class EponOnuPrototype final : public Prototype
{
public:
    EponOnuPrototype(std::string mode, int tag) : Prototype(mode), onu_tag_(tag)
    {
        std::cout << "EponOnuPrototype default constructor" << std::endl;
    }
    EponOnuPrototype(const EponOnuPrototype &prototype) : Prototype(prototype.mode_), onu_tag_(prototype.onu_tag_)
    {
        std::cout << "EponOnuPrototype copy constructor" << std::endl;
    }

    std::shared_ptr<Prototype> Clone() const override
    {
        return std::make_shared<EponOnuPrototype>(*this);
    }

private:
    int onu_tag_;
};

class XgsOnuPrototype final : public Prototype
{
public:
    XgsOnuPrototype(std::string mode, int tag) : Prototype(mode), onu_stag_(tag)
    {
        std::cout << "XgsOnuPrototype default constructor" << std::endl;
    }
    XgsOnuPrototype(const XgsOnuPrototype &prototype) : Prototype(prototype.mode_), onu_stag_(prototype.onu_stag_)
    {
        std::cout << "XgsOnuPrototype copy constructor" << std::endl;
    }

    std::shared_ptr<Prototype> Clone() const override
    {
        return std::make_shared<XgsOnuPrototype>(*this);
    }

private:
    int onu_stag_;
};

class PrototypeFactory
{
public:
    PrototypeFactory() : prototypes_{{OnuPrototype::EPON, std::make_shared<EponOnuPrototype>("Epon", 223)},
                                     {OnuPrototype::XGS, std::make_shared<XgsOnuPrototype>("Xgs", 218)}} {}

    std::shared_ptr<Prototype> CreatePrototype(OnuPrototype type)
    {
        return prototypes_[type];
    }

private:
    std::unordered_map<OnuPrototype, std::shared_ptr<Prototype>, std::hash<int>> prototypes_;
};

int main()
{
    PrototypeFactory prototypeFactory;

    std::cout << "Prototype 1" << std::endl
              << std::endl;

    auto eponPrototype = prototypeFactory.CreatePrototype(OnuPrototype::EPON);
    eponPrototype->Start(220);

    std::cout << std::endl
              << "Prototype 2" << std::endl
              << std::endl;

    auto xgsPrototype = prototypeFactory.CreatePrototype(OnuPrototype::XGS);
    xgsPrototype->Start(330);

    return 0;
}