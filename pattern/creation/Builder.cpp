/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Builder pattern to get rid of a “telescoping constructor”(constructor with a lot of parameters or a lot of constructors with a different set of parameters).

    Use the Builder pattern when you want your code to be able to create different representations of some product (for example, stone and wooden houses).

    Use the Builder to construct Composite trees or other complex objects.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    You can construct objects step-by-step, defer construction steps or run steps recursively.

    You can reuse the same construction code when building various representations of products.

    Single Responsibility Principle. You can isolate complex construction code from the business logic of the product.
Cons:
    The overall complexity of the code increases since the pattern requires creating multiple new classes.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <memory>

class Bng
{
public:
    Bng() : param1(), param2(), param3(), param4() { std::cout << "Bng constructor" << std::endl; }
    /*
        Bng(int par1, bool par2 , a lot of params) {}
        Bng(int par2, bool par5 , a lot of params) {}
        Bng(int par13, bool par7 , a lot of params) {}

        A lot of the constructors with the different combinations
    */
    virtual ~Bng() = default;

    void Start() const
    {
        std::cout << "Bng start! Parameters: " << std::boolalpha << param1 << ", " << param2 << ", " << param3 << ", " << param4 << std::endl;
    }
    /*
    Variant 1:
        void SetParams1();
        void SetParams2();
        void SetParams3();

        A lot of the setters

    Variant 2:
        friend class Builder;

        Can directly set the parameters of Bng class

    Variant 3:

        class Builder : public Bng {};

        I am not sure about this!
    */

private:
    int param1;
    bool param2;
    int param3;
    double param4;
    /* a lot of parameters*/

    friend class ThriftBuilder;
    friend class GrpcBuilder;
};

class Builder
{
public:
    Builder() : bng_(std::make_shared<Bng>()) {}
    virtual ~Builder() = default;

    virtual std::shared_ptr<Bng> GetBng() = 0;
    virtual void Reset() = 0;

    virtual void BuildNetworking() = 0;
    virtual void BuildConnections() = 0;
    virtual void BuildMulticast() = 0;

    /*
        virtual std::shared_ptr<Bng> GetBng();

        Different concrete builders can get the different products, not only the Bng
        If we have single product - can use this interface
    */
protected:
    std::shared_ptr<Bng> bng_;
};

class ThriftBuilder final : public Builder
{
public:
    ThriftBuilder()
    {
        std::cout << "ThriftBuilder" << std::endl;
    }

    std::shared_ptr<Bng> GetBng() override { return bng_; }

    void Reset() override
    {
        bng_.reset(new Bng());
    }

    void BuildNetworking() override
    {
        std::cout << "ThriftBuilder: build networking" << std::endl;
        bng_->param1 = 225;
        // set another parameters
    }

    void BuildConnections() override
    {
        std::cout << "ThriftBuilder: build connections" << std::endl;
        bng_->param2 = true;
        // set another parameters
    }
    void BuildMulticast() override
    {
        std::cout << "ThriftBuilder: build multicast" << std::endl;
        bng_->param4 = 7.7;
        // set another parameters
    }
};

class GrpcBuilder final : public Builder
{
public:
    GrpcBuilder()
    {
        std::cout << "GrpcBuilder" << std::endl;
    }

    std::shared_ptr<Bng> GetBng() override { return bng_; }

    void Reset() override
    {
        bng_.reset(new Bng());
    }

    void BuildNetworking() override
    {
        std::cout << "GrpcBuilder: build networking" << std::endl;
        bng_->param1 = 123;
        // set another parameters
    }

    void BuildConnections() override
    {
        std::cout << "GrpcBuilder: build connections" << std::endl;
        bng_->param2 = false;
        // set another parameters
    }
    void BuildMulticast() override
    {
        std::cout << "GrpcBuilder: build multicast" << std::endl;
        bng_->param3 = 9;
        bng_->param4 = 9.3;
        // set another parameters
    }
};

class Director final
{
public:
    Director() : builder_() {}
    Director(std::shared_ptr<Builder> builder) : builder_(builder) {}
    void SetBuilder(std::shared_ptr<Builder> builder) { builder_ = builder; }

    void BuildConnection()
    {
        if (!builder_)
        {
            std::cerr << "Error: Builder is not set!" << std::endl;
            return;
        }

        std::cout << "Director: build connection" << std::endl;

        builder_->BuildConnections();
    }

    void BuildFullDuplex()
    {
        if (!builder_)
        {
            std::cerr << "Error: Builder is not set!" << std::endl;
            return;
        }

        std::cout << "Director: build full duplex" << std::endl;

        builder_->BuildConnections();
        builder_->BuildMulticast();
    }

    void BuildAll()
    {
        if (!builder_)
        {
            std::cerr << "Error: Builder is not set!" << std::endl;
            return;
        }

        std::cout << "Director: build all" << std::endl;

        builder_->BuildNetworking();
        builder_->BuildConnections();
        builder_->BuildMulticast();
    }

private:
    std::shared_ptr<Builder> builder_;
};

int main()
{
    std::cout << std::endl
              << "Builder 1" << std::endl
              << std::endl;

    std::shared_ptr<Director> director = std::make_shared<Director>();

    std::shared_ptr<Builder> thriftBuilder = std::make_shared<ThriftBuilder>();

    director->SetBuilder(thriftBuilder);
    director->BuildAll();
    auto thriftBng = thriftBuilder->GetBng();
    thriftBng->Start();

    thriftBuilder->Reset();
    director->BuildFullDuplex();
    auto thriftDuplexBng = thriftBuilder->GetBng();
    thriftDuplexBng->Start();

    std::cout << std::endl
              << "Builder 2" << std::endl
              << std::endl;

    std::shared_ptr<Builder> grpcBuilder = std::make_shared<GrpcBuilder>();

    director->SetBuilder(grpcBuilder);
    director->BuildAll();
    auto grpcBng = grpcBuilder->GetBng();
    grpcBng->Start();

    grpcBuilder->Reset();
    director->BuildConnection();
    auto grpcConBng = grpcBuilder->GetBng();
    grpcConBng->Start();
}