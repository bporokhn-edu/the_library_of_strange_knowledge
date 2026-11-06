/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Adapter class when you want to use some existing class, but its interface isn’t compatible with the rest of your code.

    Use the pattern when you want to reuse several existing subclasses that lack some common functionality that can’t be added to the superclass.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    Single Responsibility Principle.
    You can separate the interface or data conversion code from the primary business logic of the program.

    Open/Closed Principle.
    You can introduce new types of adapters into the program without breaking the existing client code, as long as they work with the adapters through the client interface.

Cons:
    The overall complexity of the code increases because you need to introduce a set of new interfaces and classes.
    Sometimes it’s simpler just to change the service class so that it matches the rest of your code.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <memory>

typedef const std::string &JSON;
typedef void ParsedData;
typedef void ParsedThriftData;

// 3rd party class, can't change
class ParserBcm final
{
public:
    ParsedData ParseTlv11(const std::string &buffer)
    {
        std::cout << "ParserBcm: ParseTlv11: " << buffer << std::endl;
    }
    ParsedData ParseTlv53(const std::string &buffer)
    {
        std::cout << "ParserBcm: ParseTlv53: " << buffer << std::endl;
    }
    ParsedData ParseBase(const std::string &buffer)
    {
        std::cout << "ParserBcm: ParseBase: " << buffer << std::endl;
    }
};

class ParserThrift
{
public:
    virtual ~ParserThrift() = default;

    ParsedThriftData ParseBuffer(const std::string &buffer)
    {
        std::cout << "ParserThrift: ParseBuffer: " << buffer << std::endl;
    }
};

class IParser
{
public:
    virtual ~IParser() = default;

    virtual void Parse(JSON json) = 0;
};

class TrParser : public IParser
{
public:
    ParsedData Parse(JSON json) override
    {
        std::cout << "TrParser: Parse: " << json << std::endl;
    }
};

class BcmParserAdapter : public IParser
{
public:
    BcmParserAdapter(std::shared_ptr<ParserBcm> parserBcm) : parserBcm_(parserBcm) {}

    ParsedData Parse(JSON json) override
    {
        std::cout << "BcmParserAdapter: convert to buffer: " << json << std::endl;
        auto buffer = "\"" + json.substr(7, json.size() - 8); // imitation of converting types
        std::cout << "BcmParserAdapter: Parse: " << buffer << std::endl;
        parserBcm_->ParseBase(buffer);
        parserBcm_->ParseTlv11(buffer);
        parserBcm_->ParseTlv53(buffer);
    }

private:
    std::shared_ptr<ParserBcm> parserBcm_;
};

class ThriftParserAdapter : public IParser, public ParserThrift
{
public:
    ParsedData Parse(JSON json) override
    {
        std::cout << "ThriftParserAdapter: convert to buffer: " << json << std::endl;
        auto buffer = "\"" + json.substr(7, json.size() - 8); // imitation of converting types
        std::cout << "ThriftParserAdapter: Parse: " << buffer << std::endl;
        ParseBuffer(buffer);
        // convert ParsedThriftData to ParsedData
    }
};

int main()
{
    std::cout << "Adapter 1" << std::endl
              << std::endl;

    JSON json = "{\"JSON Text from config file\"}";

    std::shared_ptr<IParser> trParser = std::make_shared<TrParser>();
    trParser->Parse(json);

    std::shared_ptr<ParserBcm> parserBcm = std::make_shared<ParserBcm>();
    // ParserBcm can't parse JSON
    std::shared_ptr<IParser> bcmParserAdapter = std::make_shared<BcmParserAdapter>(parserBcm);

    bcmParserAdapter->Parse(json);

    std::cout << std::endl
              << "Adapter 2" << std::endl
              << std::endl;

    std::shared_ptr<IParser> thriftParserAdapter = std::make_shared<ThriftParserAdapter>();

    thriftParserAdapter->Parse(json);

    return 0;
}