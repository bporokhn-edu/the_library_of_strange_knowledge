/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Visitor
    is a behavioral design pattern that lets you separate algorithms from the objects on which they operate.

When:
    Use the Visitor when you need to perform an operation on all elements of a complex object structure
    (for example, an object tree).

    Use the Visitor to clean up the business logic of auxiliary behaviors.

    Use the pattern when a behavior makes sense only in some classes of a class hierarchy, but not in others.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    Open/Closed Principle.
    You can introduce a new behavior that can work with objects of different classes without changing these classes.

    Single Responsibility Principle.
    You can move multiple versions of the same behavior into the same class.

    A visitor object can accumulate some useful information while working with various objects.
    This might be handy when you want to traverse some complex object structure, such as an object tree,
    and apply the visitor to each object of this structure.

Cons:
    You need to update all visitors each time a class gets added to or removed from the element hierarchy.

    Visitors might lack the necessary access to the private fields and methods of the elements that they’re supposed to work with.

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

class Visitor;

class SubElement
{
public:
    virtual ~SubElement() = default;
    virtual void Apply(Visitor &visitor) = 0;
};

class Element
{
public:
    Element(const std::vector<std::shared_ptr<SubElement>> &subElements) : subElements_(subElements) {}
    virtual ~Element() = default;
    virtual void Apply(Visitor &visitor) = 0;

protected:
    std::vector<std::shared_ptr<SubElement>> subElements_{};
};

class ConcreteElementA;
class ConcreteElementB;

class ConcreteSubElementA;
class ConcreteSubElementB;

class Visitor
{
public:
    virtual ~Visitor() = default;
    virtual void visitConcreteElementA(ConcreteElementA &element) = 0;
    virtual void visitConcreteElementB(ConcreteElementB &element) = 0;
    virtual void visitConcreteSubElementA(ConcreteSubElementA &element) = 0;
    virtual void visitConcreteSubElementB(ConcreteSubElementB &element) = 0;
};

class ConcreteElementA : public Element
{
public:
    ConcreteElementA(const std::vector<std::shared_ptr<SubElement>> &subElements) : Element(subElements) {}
    void Apply(Visitor &visitor) override
    {
        visitor.visitConcreteElementA(*this);
        for(auto subElement: subElements_) {
            subElement->Apply(visitor);
        }
    }
    void operationA() const
    {
        std::cout << "Operation A" << std::endl;
    }
};

class ConcreteElementB : public Element
{
public:
    ConcreteElementB(const std::vector<std::shared_ptr<SubElement>> &subElements) : Element(subElements) {}
    void Apply(Visitor &visitor) override
    {
        visitor.visitConcreteElementB(*this);
        for(auto subElement: subElements_) {
            subElement->Apply(visitor);
        }
    }
    void operationB() const
    {
        std::cout << "Operation B" << std::endl;
    }
};

class ConcreteSubElementA : public SubElement
{
public:
    void Apply(Visitor &visitor) override
    {
        visitor.visitConcreteSubElementA(*this);
    }
    void operationA() const
    {
        std::cout << "SubOperation A" << std::endl;
    }
};

class ConcreteSubElementB : public SubElement
{
public:
    void Apply(Visitor &visitor) override
    {
        visitor.visitConcreteSubElementB(*this);
    }
    void operationB() const
    {
        std::cout << "SubOperation B" << std::endl;
    }
};

class ConcreteVisitor : public Visitor
{
public:
    void visitConcreteElementA(ConcreteElementA &element) override
    {
        std::cout << "Visiting ConcreteElementA" << std::endl;
        element.operationA();
    }

    void visitConcreteElementB(ConcreteElementB &element) override
    {
        std::cout << "Visiting ConcreteElementB" << std::endl;
        element.operationB();
    }

    void visitConcreteSubElementA(ConcreteSubElementA &element) override
    {
        std::cout << "Visiting ConcreteSubElementA" << std::endl;
        element.operationA();
    }

    void visitConcreteSubElementB(ConcreteSubElementB &element) override
    {
        std::cout << "Visiting ConcreteSubElementB" << std::endl;
        element.operationB();
    }
};

int main()
{
    std::cout << std::endl
              << "Visitor 1" << std::endl
              << std::endl;


    std::vector<std::shared_ptr<SubElement>> subElements = {std::make_shared<ConcreteSubElementA>(), std::make_shared<ConcreteSubElementB>()};
    std::vector<std::shared_ptr<Element>> elements = {std::make_shared<ConcreteElementA>(subElements), std::make_shared<ConcreteElementB>(subElements)};
    ConcreteVisitor visitor;

    for (auto element : elements)
    {
        element->Apply(visitor);
    }

    std::cout << std::endl
              << "Visitor end" << std::endl
              << std::endl;

    return 0;
}
