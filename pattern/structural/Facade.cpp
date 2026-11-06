/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Facade pattern when you need to have a limited but straightforward interface to a complex subsystem.

    Use the Facade when you want to structure a subsystem into layers.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    You can isolate your code from the complexity of a subsystem.

Cons:
    A facade can become a god object coupled to all classes of an app.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <memory>

enum ServerMode
{
    ThriftServerMode,
    RpcServerMode
};

class ThriftServer
{
public:
    void Reboot()
    {
        std::cout << "ThriftServer: Reboot" << std::endl;
    }
    void Restart()
    {
        std::cout << "ThriftServer: Restart" << std::endl;
    }
};

class RpcServer
{
public:
    void Reboot()
    {
        std::cout << "RpcServer: Reboot" << std::endl;
    }
    void Reconnect()
    {
        std::cout << "RpcServer: Reconnect" << std::endl;
    }
};

class Facade
{
public:
    Facade(std::shared_ptr<ThriftServer> thriftServer, std::shared_ptr<RpcServer> rpcServer) : thriftServer_(thriftServer), rpcServer_(rpcServer) {}

    void Reboot(ServerMode serverMode = ServerMode::ThriftServerMode)
    {
        std::cout << "Facade Reboot: server mode = " << (serverMode == ServerMode::ThriftServerMode ? "ThriftServerMode" : "RpcServerMode") << std::endl;

        switch (serverMode)
        {
        case ServerMode::ThriftServerMode:
            thriftServer_->Restart();
            thriftServer_->Reboot();
            break;
        case ServerMode::RpcServerMode:
            rpcServer_->Reconnect();
            rpcServer_->Reboot();
            break;
        default:
            std::cout << "Unsupported Server Mode" << std::endl;
        }
    }

protected:
    std::shared_ptr<ThriftServer> thriftServer_;
    std::shared_ptr<RpcServer> rpcServer_;
};

int main()
{
    std::cout << "Facade 1" << std::endl
              << std::endl;

    std::shared_ptr<ThriftServer> thriftServer = std::make_shared<ThriftServer>();
    std::shared_ptr<RpcServer> rpcServer = std::make_shared<RpcServer>();

    Facade facade(thriftServer, rpcServer);

    facade.Reboot();

    std::cout << "Facade 2" << std::endl
              << std::endl;

    facade.Reboot(ServerMode::RpcServerMode);

    return 0;
}
