#include <iostream>
#include <memory>
#include <iomanip> // For std::setw, std::setprecision
#include <type_traits>
#include <tuple> // For std::tuple
#include <variant>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>

void process(const std::string &value)
{
    // Do something with value
}

std::string lvalue = "Hello";
template <typename T>
void maybe_throw(T t) noexcept(noexcept(T()))
{
    T temp = T(); // Conditional noexcept based on T's default constructor
}
void baz() noexcept { throw 42; } // noexcept is the same as noexcept(true)

template <typename... Args>
auto sum(Args... args)
{
    return (args + ...); // Unary right fold: sums all elements
}

class MyClass
{
public:
    MyClass(int a, double b)
    {
        std::cout << "My class: " << a << ", " << b << std::endl;
        // Constructor logic
    }
    MyClass()
    {
        std::cout << "My class" << std::endl;
        // Constructor logic
    }
    // Disable copy
    // MyClass(const MyClass &) = delete;
    // MyClass &operator=(const MyClass &) = delete;

    // MyClass(MyClass &&) = delete;
    // MyClass &operator=(MyClass &&) = delete;

    bool operator<(const MyClass &m) { return false; }
};

struct B
{
    int x;
    virtual void foo() {}
};

template <typename T>
void printType()
{
    std::cout << "Original type: " << typeid(T).name() << std::endl;
    std::cout << "Transformed type: " << typeid(typename std::decay<T>::type).name() << std::endl;
}

void print(int a, double b, const std::string &c)
{
    std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
}

void handleVariant(const std::variant<int, double, std::string> &v)
{
    std::visit([](auto &&arg)
               { std::cout << "Value: " << arg << std::endl; }, v);
}

// void *operator new(std::size_t size, const std::nothrow_t &tag)
// {
//     std::cout << "Custom new noexcept called, size: " << size << std::endl;
//     return malloc(size);
// }
// void *operator new(std::size_t size)
// {
//     std::cout << "Custom new called, size: " << size << std::endl;
//     return malloc(size);
// }
// void *operator new(std::size_t size, std::align_val_t al)
// {
//     std::cout << "Custom align_val_t new called, size: " << size << std::endl;
//     return malloc(size);
// }
// void operator delete(void *ptr) noexcept
// {
//     std::cout << "Custom delete called" << std::endl;
//     free(ptr);
// }

// void operator delete(void *ptr, std::align_val_t al) noexcept
// {
//     std::cout << "Custom align_val_t delete called" << std::endl;
//     free(ptr);
// }
// void operator delete(void *ptr, std::size_t sz) noexcept
// {
//     std::cout << "Custom size_t delete called = " << sz << std::endl;
//     free(ptr);
// }
struct alignas(128) A
{
    int x;
    A(int a = 10) : x(a) { std::cout << "A" << std::endl; }
    A(const A &a) {}
    ~A() { std::cout << "~A" << std::endl; }

private:
    double d;
};
// void *operator new(std::size_t size, void *ptr)
// {
//     std::cout << "Custom placement new called, size: " << size << "\n";
//     return ptr; // Return the pointer passed as argument
// }
int main()
{
    std::vector<int> vec = {1, 2, 3, 1, 2, 4, 5, 6, 3, 7};
    std::vector<int> vec2 = {11, 5, 6,11,  3, 7};

    std::cout << "All elements: ";
    for (const int &element : vec)
    {
        std::cout << element << " ";
    }
    std::cout << std::endl;
    for (const int &element : vec2)
    {
        std::cout << element << " ";
    }

    std::set<int> un;
    std::copy_if(vec.begin(), vec.end(), std::inserter(un, un.end()), [&vec2](const auto& el) {return std::find(vec2.begin(), vec2.end(), el) != vec2.end();});
    std::cout << std::endl;
    std::cout << "Union elements: ";
    for (const int &element : un)
    {
        std::cout << element << " ";
    }
    std::map<int, int> elementCount; // Map to count occurrences of each element

    // Count the occurrences of each element in the vector
    // for (const int &element : vec)
    // {
    //     elementCount[element]++;
    // }

    std::for_each(vec.begin(), vec.end(), [&elementCount](const auto &element)
                  { elementCount[element]++; });
    std::vector<int> uniqueElements;
    std::vector<int> nonUniqueElements;


    // Separate unique and non-unique elements
    for (const auto &[key, count] : elementCount)
    {
        if (count == 1)
        {
            uniqueElements.push_back(key);
        }
        else
        {
            nonUniqueElements.push_back(key);
        }
    }

    // Print unique elements
    std::cout << "Unique elements: ";
    for (const int &element : uniqueElements)
    {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    // Print non-unique elements
    std::cout << "Non-unique elements: ";
    for (const int &element : nonUniqueElements)
    {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    return 0;
    std::vector<MyClass> v1;
    MyClass m;
    v1.push_back(m);
    A *aaa = (A *)(malloc(sizeof(A)));
    new (aaa) A(10);

    aaa->~A();
    free(aaa);

    // A *aaa = new A;
    // delete aaa;

    std::cout << "END = " << __STDCPP_DEFAULT_NEW_ALIGNMENT__ << std::endl;
    return 0;
    // std::variant<int, double, std::string> v = "Hello";
    std::variant<int, double, std::string> v;

    handleVariant(v); // Outputs: Value: Hello

    v = 3.14;
    handleVariant(v); // Outputs: Value: 3.14

    v = 42;
    handleVariant(v); // Outputs: Value: 42

    auto t = std::make_tuple(1, 3.14, "Hello");

    std::apply(print, t); // Unpacks the tuple and calls print function

    std::cout << "First: " << std::get<0>(t) << ", Second: " << std::get<1>(t) << ", Third: " << std::get<2>(t) << std::endl;

    int a1 = 10;
    const int &b = a1;

    const int ar[] = {1, 2, 4};
    printType<decltype(b)>(); // Original: const int&, Transformed: int
    printType<decltype(ar)>();

    std::cout << "Size of A: " << sizeof(A) << std::endl;                                                  // Size of A
    std::cout << "Size of B: " << sizeof(B) << std::endl;                                                  // Size of B
    std::cout << "Is A trivially copyable? " << std::is_trivially_copyable<A>::value << std::endl;         // true
    std::cout << "Is B trivially copyable? " << std::is_trivially_copyable<const B &>::value << std::endl; // false

    double pi = 3.14159;
    std::cout << std::fixed << std::setprecision(2); // Set fixed format with 2 decimal places
    std::cout << "Pi is approximately: " << pi << std::endl;

    std::cout << std::setw(10) << "Number" << std::endl;
    std::cout << std::setw(10) << 42 << std::endl; // Output number with width of 10

    double number = 12345.6789;

    // Output in fixed-point notation
    std::cout << std::fixed << std::setprecision(2); // Set precision to 2 decimal places
    std::cout << "Fixed-point notation: " << number << std::endl;

    // Output in scientific notation
    std::cout << std::scientific << std::setprecision(3); // Set precision to 3 decimal places
    std::cout << "Scientific notation: " << number << std::endl;

    std::shared_ptr<MyClass> sp = std::make_shared<MyClass>();
    std::cout << sum(10, 4.3, (float)2.1) << std::endl;
    // Passing different value categories:
    int a = 8;
    int &r = a;
    const int &cref = 42; // Const reference to an rvalue

    process(lvalue);             // Passing an lvalue
    process("Temporary String"); // Passing a prvalue (string literal)
    process(std::move(std::move(lvalue)));
    process(std::move("lala"));
    std::cout << "Unscoped enumeration" << std::endl;
    {
        enum Color
        {
            red,
            green,
            blue
        };
        Color r = red;

        switch (r)
        {
        case red:
            std::cout << "red\n";
            break;
        case green:
            std::cout << "green\n";
            break;
        case blue:
            std::cout << "blue\n";
            break;
        }
    }
    {
        enum Foo
        {
            a,
            b,
            c = 10,
            d,
            e = 1,
            f,
            g = f + c
        };
        // a = 0, b = 1, c = 10, d = 11, e = 1, f = 2, g = 12
    }
    {
        enum
        {
            a,
            b,
            c = 0,
            d = a + 2
        }; // defines a = 0, b = 1, c = 0, d = 2
    }
    {
        struct X
        {
            enum direction
            {
                left = 'l',
                right = 'r'
            };
        };
        X x;
        X *p = &x;

        [[maybe_unused]] int a = X::direction::left; // allowed only in C++11 and later
        [[maybe_unused]] int b = X::left;
        [[maybe_unused]] int c = x.left;
        [[maybe_unused]] int d = p->left;
    }
    {
        struct S
        {
            enum E1 : int
            {
            };
            // enum E1 : int
            // {
            // }; // error: redeclaration of enumeration,
            // NOT parsed as a zero-length bit-field of type enum E1
        };

        enum E2
        {
            e1
        };

        [[maybe_unused]] auto f = []()
        {
            false ? new E2 : int{}; // OK: 'int' is NOT parsed as the underlying type
        };
    }

    return 0;
}