/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Lazy initialization (virtual proxy). This is when you have a heavyweight service object
    that wastes system resources by being always up, even though you only need it from time to time.

    Access control (protection proxy). This is when you want only specific clients to be able to use the service object;
    for instance, when your objects are crucial parts of an operating system and
    clients are various launched applications (including malicious ones).

    Local execution of a remote service (remote proxy). This is when the service object is located on a remote server.

    Logging requests (logging proxy). This is when you want to keep a history of requests to the service object.

    Caching request results (caching proxy). This is when you need to cache results of client requests and
    manage the life cycle of this cache, especially if results are quite large.

    Smart reference.
    This is when you need to be able to dismiss a heavyweight object once there are no clients that use it.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    You can control the service object without clients knowing about it.

    You can manage the lifecycle of the service object when clients don’t care about it.

    The proxy works even if the service object isn’t ready or is not available.

    Open/Closed Principle.
    You can introduce new proxies without changing the service or clients.

Cons:
    The code may become more complicated since you need to introduce a lot of new classes.

    The response from the service might get delayed.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <vector>
#include <memory>
#include <ios>

const char clientDns[] = "pod-name.dns";

class Connection
{
public:
    virtual ~Connection() = default;
    virtual void Connect() = 0;
    virtual bool IsConnected() = 0;
    virtual void Request() = 0;
};

class ThriftConnection : public Connection
{
public:
    void Connect() override
    {
        std::cout << "ThriftConnection: Connect" << std::endl;
        isConnected = true;
    }
    bool IsConnected() override
    {
        std::cout << "ThriftConnection: IsConnected" << std::endl;
        return isConnected;
    }
    void Request()
    {
        std::cout << "ThriftConnection: Request" << std::endl;
    }

private:
    bool isConnected = false;
};

class ThriftLazyConnection : public Connection
{
public:
    ThriftLazyConnection(std::shared_ptr<ThriftConnection> thriftConnection = nullptr) : thriftConnection_(thriftConnection) {}

    void Connect() override
    {
        std::cout << "ThriftLazyConnection: Connect" << std::endl;
        if (IsConnected())
        {
            std::cout << "ThriftLazyConnection is already connected" << std::endl;
            return;
        }

        thriftConnection_->Connect();
    }

    bool IsConnected() override
    {
        std::cout << "ThriftLazyConnection: IsConnected" << std::endl;
        return thriftConnection_->IsConnected();
    }

    void Request()
    {
        std::cout << "ThriftLazyConnection: Request" << std::endl;
        if (!IsConnected())
        {
            std::cout << "ThriftLazyConnection: is not connected" << std::endl;
            Connect();
        }
        thriftConnection_->Request();
    }

private:
    std::shared_ptr<ThriftConnection> thriftConnection_;
};

int main()
{
    std::cout << "Proxy 1" << std::endl
              << std::endl;

    std::shared_ptr<Connection> connection = std::make_shared<ThriftConnection>();

    connection->Connect();
    bool isCon = connection->IsConnected();
    std::cout << "Is connected = " << std::boolalpha << isCon << std::endl;
    connection->Request();

    std::cout << std::endl
              << "Proxy 2" << std::endl
              << std::endl;

    std::shared_ptr<Connection> lazyConnection = std::make_shared<ThriftLazyConnection>(std::make_shared<ThriftConnection>());

    lazyConnection->Request();

    isCon = lazyConnection->IsConnected();
    std::cout << "Is connected = " << std::boolalpha << isCon << std::endl;

    lazyConnection->Connect();

    return 0;
}
