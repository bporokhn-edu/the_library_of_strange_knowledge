/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Template Method
    is a behavioral design pattern that defines the skeleton of an algorithm in the superclass
    but lets subclasses override specific steps of the algorithm without changing its structure.

When:
    Use the Template Method pattern when you want to let clients extend only particular steps of an algorithm,
    but not the whole algorithm or its structure.

    Use the pattern when you have several classes that contain almost identical algorithms with some minor differences.
    As a result, you might need to modify all classes when the algorithm changes.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    You can let clients override only certain parts of a large algorithm,
    making them less affected by changes that happen to other parts of the algorithm.

    You can pull the duplicate code into a superclass.

Cons:
    Some clients may be limited by the provided skeleton of an algorithm.

    You might violate the Liskov Substitution Principle by suppressing a default step implementation via a subclass.

    Template methods tend to be harder to maintain the more steps they have.

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

class DocumentGenerator
{
public:
    virtual ~DocumentGenerator() = default;

    // Template method
    void generateDocument()
    {
        openDocument();
        addContent();
        closeDocument();
    }

protected:
    virtual void openDocument() const = 0;
    virtual void addContent() const = 0;
    virtual void closeDocument() const = 0;
};

class ReportGenerator : public DocumentGenerator
{
protected:
    void openDocument() const override
    {
        std::cout << "ReportGenerator: Opening report document..." << std::endl;
    }

    void addContent() const override
    {
        std::cout << "ReportGenerator: Adding report content..." << std::endl;
    }

    void closeDocument() const override
    {
        std::cout << "ReportGenerator: Closing report document." << std::endl;
    }
};

class InvoiceGenerator : public DocumentGenerator
{
protected:
    void openDocument() const override
    {
        std::cout << "InvoiceGenerator: Opening invoice document..." << std::endl;
    }

    void addContent() const override
    {
        std::cout << "InvoiceGenerator: Adding invoice content..." << std::endl;
    }

    void closeDocument() const override
    {
        std::cout << "InvoiceGenerator: Closing invoice document." << std::endl;
    }
};

class InvoiceGeneratorPro : public InvoiceGenerator
{
protected:
    void addContent() const override
    {
        std::cout << "InvoiceGeneratorPro: Pro Adding invoice content..." << std::endl;
    }
};

void ClientCode(std::shared_ptr<DocumentGenerator> docGen)
{
    std::cout << "Generating report:" << std::endl;
    docGen->generateDocument();
    std::cout << std::endl;
}

int main()
{
    std::cout << std::endl
              << "Template Method 1" << std::endl
              << std::endl;

    std::shared_ptr<DocumentGenerator> report = std::make_shared<ReportGenerator>();
    ClientCode(report);

    std::shared_ptr<DocumentGenerator> invoice = std::make_shared<InvoiceGenerator>();
    ClientCode(invoice);

    std::shared_ptr<DocumentGenerator> invoicePro = std::make_shared<InvoiceGeneratorPro>();
    ClientCode(invoicePro);

    std::cout << std::endl
              << "Template Method end" << std::endl
              << std::endl;

    return 0;
}
