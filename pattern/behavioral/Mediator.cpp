/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Mediator pattern when it’s hard to change some of the classes because
    they are tightly coupled to a bunch of other classes.

    Use the pattern when you can’t reuse a component in a different program because it’s too dependent on other components.

    Use the Mediator when you find yourself creating tons of component subclasses just to
    reuse some basic behavior in various contexts.


* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    Single Responsibility Principle.
    You can extract the communications between various components into a single place, making it easier to comprehend and maintain.

    Open/Closed Principle.
    You can introduce new mediators without having to change the actual components.

    You can reduce coupling between various components of a program.

    You can reuse individual components more easily.

Cons:
    Over time a mediator can evolve into a God Object.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <vector>
#include <memory>
#include <ios>
#include <algorithm>
#include <functional>
#include <variant>
#include <exception>
#include <stdexcept>

enum class MediatorEvents
{
    ProvisionOnu,    // Mgr2Spa
    ProvisionOnuRes, // Spa2Mgr

    RegistreCpe,        // Bng2Spa
    RegistreCpeProv,    // Spa2Mgr
    RegistreCpeProvRes, // Mgr2Spa
    RegistreCpeRes,     // Spa2Bng
};

struct ProvisioningData
{
    std::string str;
};

struct ProvisioningResponseData
{
    std::string str;
};

struct RegisterCpeData
{
    std::string str;
};

typedef std::variant<ProvisioningData, ProvisioningResponseData, RegisterCpeData> Data;

typedef std::function<void(bool)> ResultCb;
ResultCb defaultCb = []([[maybe_unused]] bool res) {};

class IMediator
{
public:
    virtual ~IMediator() = default;
    virtual void Notify(MediatorEvents, const Data &, ResultCb cb = defaultCb) = 0;
};

class Mgr
{
    std::shared_ptr<IMediator> mediator_{};

public:
    Mgr()
    {
        std::cout << "Mgr created" << std::endl;
    }
    void SetMediator(std::shared_ptr<IMediator> mediator)
    {
        mediator_ = mediator;
    }

    void ProvisionOnu(const ProvisioningData &data)
    {
        std::cout << "Mgr: ProvisionOnu. Data = " << data.str << std::endl;
        Data variantData(data);
        auto cb = [](bool res)
        { std::cout << "Mgr Onu Provisioned. Res = " << res << std::endl; };

        mediator_->Notify(MediatorEvents::ProvisionOnu, data, cb);
    }

    void ProvisionOnuResponse(const ProvisioningResponseData &data)
    {
        std::cout << "Mgr: ProvisioningResponseData. Data = " << data.str << std::endl;
    }

    void RegisterCpeProv(const RegisterCpeData &data, ResultCb cb)
    {
        std::cout << "Mgr: RegisterCpeProv. Data = " << data.str << std::endl;
        cb(true);

        mediator_->Notify(MediatorEvents::RegistreCpeProvRes, data);
    }
};

class Spa
{
    std::shared_ptr<IMediator> mediator_{};

public:
    Spa()
    {
        std::cout << "Spa created" << std::endl;
    }

    void SetMediator(std::shared_ptr<IMediator> mediator)
    {
        mediator_ = mediator;
    }
    void ProvisionOnu(const ProvisioningData &data, ResultCb cb)
    {
        std::cout << "Spa: ProvisionOnu. Data = " << data.str << std::endl;
        cb(true);
    }

    void RegisterCpe(const RegisterCpeData &data, ResultCb cb)
    {
        std::cout << "Spa: RegisterCpe. Data = " << data.str << std::endl;

        mediator_->Notify(MediatorEvents::RegistreCpeProv, data, cb);
    }

    void RegisterCpeProvRes(const RegisterCpeData &data, ResultCb cb)
    {
        std::cout << "Spa: RegisterCpeProvRes. Data = " << data.str << std::endl;
        cb(true);

        auto cbRes = [data](bool res)
        { std::cout << "Spa: Cpe is online. Res = " << res << " , data = " << data.str << std::endl; };

        mediator_->Notify(MediatorEvents::RegistreCpeRes, data, cbRes);
    }
};

class Bng
{
    std::shared_ptr<IMediator> mediator_{};

public:
    Bng()
    {
        std::cout << "Bng created" << std::endl;
    }

    void SetMediator(std::shared_ptr<IMediator> mediator)
    {
        mediator_ = mediator;
    }

    void RegisterCpe(const RegisterCpeData &data)
    {
        std::cout << "Bng: RegisterCpe. Data = " << data.str << std::endl;
        auto cb = [data](bool res)
        { std::cout << "Bng Cpe registered. Res = " << res << " , data = " << data.str << std::endl; };

        mediator_->Notify(MediatorEvents::RegistreCpe, data, cb);
    }

    void RegisterCpeRes(const RegisterCpeData &data, ResultCb cb)
    {
        std::cout << "Bng: RegisterCpeRes. Data = " << data.str << std::endl;
        cb(true);
        std::cout << "Bng: Cpe is online" << std::endl;
    }
};

class Mediator : public IMediator
{
    std::shared_ptr<Mgr> mgr_;
    std::shared_ptr<Spa> spa_;
    std::shared_ptr<Bng> bng_;

public:
    Mediator(const std::shared_ptr<Mgr> &mgr, const std::shared_ptr<Spa> &spa, const std::shared_ptr<Bng> &bng) : mgr_(mgr), spa_(spa), bng_(bng)
    {
        std::cout << "Mediator created" << std::endl;
    }

    void Notify(MediatorEvents event, const Data &data, ResultCb cb = defaultCb) override
    {
        std::cout << "Mediator: Notify. Event = " << (int)event << std::endl;

        try
        {
            switch (event)
            {
            case MediatorEvents::ProvisionOnu:
            {
                std::cout << "Mediator: ProvisionOnu" << std::endl;

                ProvisioningData provData = std::get<ProvisioningData>(data);
                spa_->ProvisionOnu(provData, cb);
            }
            break;
            case MediatorEvents::ProvisionOnuRes:
            {
                std::cout << "Mediator: ProvisionOnuRes" << std::endl;

                ProvisioningResponseData provData = std::get<ProvisioningResponseData>(data);
                mgr_->ProvisionOnuResponse(provData);
            }
            break;
            case MediatorEvents::RegistreCpe:
            {
                std::cout << "Mediator: RegistreCpe" << std::endl;

                RegisterCpeData regData = std::get<RegisterCpeData>(data);
                spa_->RegisterCpe(regData, cb);
            }
            break;
            case MediatorEvents::RegistreCpeProv:
            {
                std::cout << "Mediator: RegistreCpeProv" << std::endl;

                RegisterCpeData regData = std::get<RegisterCpeData>(data);
                mgr_->RegisterCpeProv(regData, cb);
            }
            break;
            case MediatorEvents::RegistreCpeProvRes:
            {
                std::cout << "Mediator: RegistreCpeProvRes" << std::endl;

                RegisterCpeData regData = std::get<RegisterCpeData>(data);
                spa_->RegisterCpeProvRes(regData, cb);
            }
            break;
            case MediatorEvents::RegistreCpeRes:
            {
                std::cout << "Mediator: RegistreCpeRes" << std::endl;

                RegisterCpeData regData = std::get<RegisterCpeData>(data);
                bng_->RegisterCpeRes(regData, cb);
            }
            break;
            default:
                break;
            }
        }
        catch (const std::bad_variant_access &ex)
        {
            std::cout << "Invalid data type: " << ex.what() << '\n';
        }
        catch (const std::exception &e)
        {
            std::cout << "Unknown Exception: " << e.what() << std::endl;
        }
    }
};

int main()
{
    std::cout << std::endl
              << "Mediator 1" << std::endl
              << std::endl;

    std::shared_ptr<Mgr> mgr = std::make_shared<Mgr>();
    std::shared_ptr<Spa> spa = std::make_shared<Spa>();
    std::shared_ptr<Bng> bng = std::make_shared<Bng>();

    std::shared_ptr<IMediator> mediator = std::make_shared<Mediator>(mgr, spa, bng);
    mgr->SetMediator(mediator);
    spa->SetMediator(mediator);
    bng->SetMediator(mediator);

    mgr->ProvisionOnu({"Onu: 12.13.14.15.16.17"});

    std::cout
        << std::endl
        << "Mediator 2" << std::endl
        << std::endl;

    bng->RegisterCpe({"Cpe: 15.21.15.4"});

    std::cout
        << std::endl
        << "Mediator End" << std::endl
        << std::endl;

    return 0;
}
