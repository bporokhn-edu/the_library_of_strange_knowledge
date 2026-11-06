/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Decorator pattern when you need to be able to assign extra behaviors to objects at runtime without breaking the code that uses these objects.

    Use the pattern when it’s awkward or not possible to extend an object’s behavior using inheritance.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    You can extend an object’s behavior without making a new subclass.

    You can add or remove responsibilities from an object at runtime.

    You can combine several behaviors by wrapping an object into multiple decorators.

    Single Responsibility Principle.
    You can divide a monolithic class that implements many possible variants of behavior into several smaller classes.

Cons:
    It’s hard to remove a specific wrapper from the wrappers stack.

    It’s hard to implement a decorator in such a way that its behavior doesn’t depend on the order in the decorators stack.

    The initial configuration code of layers might look pretty ugly.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <memory>

class IComponent
{
public:
    virtual ~IComponent() = default;

    virtual void Operation() = 0;
};

class ConcreteComponent final : public IComponent
{
public:
    void Operation() override
    {
        std::cout << "ConcreteComponent: Operation" << std::endl;
    }
};

class Decorator : public IComponent
{
public:
    Decorator(std::shared_ptr<IComponent> component) : component_(component) {}

    void Operation() override
    {
        std::cout << "Decorator: Operation" << std::endl;
        component_->Operation();
    }

protected:
    std::shared_ptr<IComponent> component_;
};

class ConcreteDecoratorA : public Decorator
{
public:
    ConcreteDecoratorA(std::shared_ptr<IComponent> component) : Decorator(component) {}

    void Operation() override
    {
        std::cout << "ConcreteDecoratorA: Operation" << std::endl;
        Decorator::Operation();
    }
};

class ConcreteDecoratorB : public Decorator
{
public:
    ConcreteDecoratorB(std::shared_ptr<IComponent> component) : Decorator(component) {}

    void Operation() override
    {
        std::cout << "ConcreteDecoratorB: Operation" << std::endl;
        Decorator::Operation();
    }
};

int main()
{
    std::cout << "Decorator 1" << std::endl
              << std::endl;

    std::shared_ptr<IComponent> simple = std::make_shared<ConcreteComponent>();
    simple->Operation();

    std::cout << std::endl
              << "Decorator 2" << std::endl
              << std::endl;

    std::shared_ptr<IComponent> decoratorA = std::make_shared<ConcreteDecoratorA>(simple);
    std::shared_ptr<IComponent> decoratorB = std::make_shared<ConcreteDecoratorB>(decoratorA);

    decoratorB->Operation();

    return 0;
}