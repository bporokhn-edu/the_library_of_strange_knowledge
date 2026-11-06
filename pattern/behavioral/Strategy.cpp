/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Strategy pattern when you want to use different variants of an algorithm within an object and
    be able to switch from one algorithm to another during runtime.

    Use the Strategy when you have a lot of similar classes that only differ in the way they execute some behavior.

    Use the pattern to isolate the business logic of a class from the implementation details of algorithms that
    may not be as important in the context of that logic.


* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    You can swap algorithms used inside an object at runtime.

    You can isolate the implementation details of an algorithm from the code that uses it.

    You can replace inheritance with composition.

    Open/Closed Principle.
    You can introduce new strategies without having to change the context.

Cons:
    If you only have a couple of algorithms and they rarely change,
    there’s no real reason to overcomplicate the program with new classes and interfaces that come along with the pattern.

    Clients must be aware of the differences between strategies to be able to select a proper one.

    A lot of modern programming languages have functional type support that lets you implement different versions of
    an algorithm inside a set of anonymous functions. Then you could use these functions exactly as you’d have used
    the strategy objects, but without bloating your code with extra classes and interfaces.

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

class PaymentStrategy
{
public:
    virtual ~PaymentStrategy() = default;
    virtual void pay(int amount) const = 0;
};

#include <iostream>

class CreditCardStrategy : public PaymentStrategy
{
public:
    void pay(int amount) const override
    {
        std::cout << "Paid " << amount << " using Credit Card." << std::endl;
    }
};

class PayPalStrategy : public PaymentStrategy
{
public:
    void pay(int amount) const override
    {
        std::cout << "Paid " << amount << " using PayPal." << std::endl;
    }
};

class BitcoinStrategy : public PaymentStrategy
{
public:
    void pay(int amount) const override
    {
        std::cout << "Paid " << amount << " using Bitcoin." << std::endl;
    }
};

class ShoppingCart
{
public:
    ShoppingCart(PaymentStrategy *strategy) : strategy_(strategy) {}

    void setStrategy(PaymentStrategy *strategy)
    {
        strategy_ = strategy;
    }

    void checkout(int amount) const
    {
        strategy_->pay(amount);
    }

private:
    PaymentStrategy *strategy_;
};

int main()
{
    std::cout << std::endl
              << "Strategy 1" << std::endl
              << std::endl;

    CreditCardStrategy creditCard;
    PayPalStrategy paypal;
    BitcoinStrategy bitcoin;

    ShoppingCart cart(&creditCard);

    cart.checkout(100); // Output: Paid 100 using Credit Card.

    cart.setStrategy(&paypal);
    cart.checkout(200); // Output: Paid 200 using PayPal.

    cart.setStrategy(&bitcoin);
    cart.checkout(300); // Output: Paid 300 using Bitcoin.

    std::cout << std::endl
              << "Strategy end" << std::endl
              << std::endl;

    return 0;
}
