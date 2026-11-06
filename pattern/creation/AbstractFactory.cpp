/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Abstract Factory when your code needs to work with various families of related
    products, but you don’t want it to depend on the concrete classes of those products—they
    might be unknown beforehand or you simply want to allow for future extensibility.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    You can be sure that the products you’re getting from a factory are compatible with each other.

    You avoid tight coupling between concrete products and client code.

    Single Responsibility Principle. You can extract the product creation code into one place,
    making the code easier to support.

    Open/Closed Principle. You can introduce new variants of products without breaking existing client code.
Cons:
    The code may become more complicated than it should be, since a lot of new interfaces and
    classes are introduced along with the pattern.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <ios>
#include <memory>

/*      Button         */
class IButton
{
public:
    virtual ~IButton() = default;

    virtual void Press(bool isDoubleClick) = 0;
};

class LinuxButton final : public IButton
{
public:
    LinuxButton(const std::string &device) : device_(device) {}

    void Press(bool isDoubleCklick) override
    {
        std::cout << "Linux button. Device: " << device_ << ". Double cklick: " << std::boolalpha << isDoubleCklick << std::endl;
    }

private:
    std::string device_;
};

class WindowsButton final : public IButton
{
public:
    WindowsButton(const std::string &controller) : controller_(controller) {}

    void Press(bool isDoubleCklick) override
    {
        std::cout << "Windows button. Controller: " << controller_ << ". Double cklick: " << std::boolalpha << isDoubleCklick << std::endl;
    }

private:
    std::string controller_;
};
/*      Button end     */

/*      ILogWindow         */
class ILogWindow
{
public:
    virtual ~ILogWindow() = default;

    virtual void Login(const std::string &user, const std::string &password) = 0;
};

class LinuxLogWindow final : public ILogWindow
{
public:
    LinuxLogWindow(int sessionId) : sessionId_(sessionId) {}

    void Login(const std::string &user, const std::string &password)
    {
        std::cout << "Linux login window. Session ID: " << sessionId_ << ". User: " << user << ", Password: " << password << std::endl;
    }

private:
    int sessionId_;
};

class WindowsLogWindow final : public ILogWindow
{
public:
    WindowsLogWindow(int loginId) : loginId_(loginId) {}

    void Login(const std::string &user, const std::string &password)
    {
        std::cout << "Windows login window. Login ID: " << loginId_ << ". User: " << user << ", Password: " << password << std::endl;
    }

private:
    int loginId_;
};
/*      ILogWindow end     */

/*      GuiFactory             */
class IGuiFactory
{
public:
    virtual ~IGuiFactory() = default;

    virtual std::shared_ptr<IButton> CreateButton() = 0;
    virtual std::shared_ptr<ILogWindow> CreateLogWindow() = 0;
};

class LinuxGuiFactory final : public IGuiFactory
{
public:
    LinuxGuiFactory(const std::string &device, int sessionId) : device_(device), sessionId_(sessionId) {}

    std::shared_ptr<IButton> CreateButton() override
    {
        std::cout << "Linux GUI factory: Create Button. Device: " << device_ << std::endl;
        return std::make_shared<LinuxButton>(device_);
    }
    std::shared_ptr<ILogWindow> CreateLogWindow()
    {
        std::cout << "Linux GUI factory: Create Log Windiw. Session ID: " << sessionId_ << std::endl;
        return std::make_shared<LinuxLogWindow>(sessionId_);
    }

private:
    std::string device_;
    int sessionId_;
};

class WindowsGuiFactory final : public IGuiFactory
{
public:
    WindowsGuiFactory(const std::string &controller, int loginId) : controller_(controller), loginId_(loginId) {}

    std::shared_ptr<IButton> CreateButton() override
    {
        std::cout << "Windows GUI factory: Create Button. Controller: " << controller_ << std::endl;
        return std::make_shared<WindowsButton>(controller_);
    }
    std::shared_ptr<ILogWindow> CreateLogWindow()
    {
        std::cout << "Windows GUI factory: Create Log Windiw. Login ID: " << loginId_ << std::endl;
        return std::make_shared<WindowsLogWindow>(loginId_);
    }

private:
    std::string controller_;
    int loginId_;
};
/*      GuiFactory end         */

/*      App         */
class App
{
public:
    App(std::shared_ptr<IGuiFactory> guiFactory) : guiFactory_(guiFactory) {}

    void Start() {
        std::cout << "App start." << std::endl;

        auto loginWindow = guiFactory_->CreateLogWindow();
        loginWindow->Login("bporokhn", "MyPassword");

        auto button = guiFactory_->CreateButton();
        button->Press(false);
    }

private:
    std::shared_ptr<IGuiFactory> guiFactory_;
};
/*      App end     */

int main()
{
    std::cout << "Abstract Factory 1" << std::endl
              << std::endl;
    
    // if (os == ubuntu)
    std::shared_ptr<IGuiFactory> guiFactory1 =  std::make_shared<LinuxGuiFactory>("Ubuntu", 1);
    // else
    // std::shared_ptr<IGuiFactory> guiFactory1 =  std::make_shared<WindowsGuiFactory>("Windows", 0);

    App app1(guiFactory1);
    app1.Start();

    std::cout << std::endl
              << "Abstract Factory 2" << std::endl
              << std::endl;

    // if (os == ubuntu)
    // std::shared_ptr<IGuiFactory> guiFactory2 =  std::make_shared<LinuxGuiFactory>("Ubuntu", 1);
    // else
    std::shared_ptr<IGuiFactory> guiFactory2 =  std::make_shared<WindowsGuiFactory>("Windows", 0);

    App app2(guiFactory2);
    app2.Start();

    return 0;
}