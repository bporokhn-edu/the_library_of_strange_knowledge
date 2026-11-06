/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Iterator pattern when your collection has a complex data structure under the hood,
    but you want to hide its complexity from clients (either for convenience or security reasons).

    Use the pattern to reduce duplication of the traversal code across your app.

    Use the Iterator when you want your code to be able to traverse different data structures or
    when types of these structures are unknown beforehand.


* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    Single Responsibility Principle.
    You can clean up the client code and the collections by extracting bulky traversal algorithms into separate classes.

    Open/Closed Principle.
    You can implement new types of collections and iterators and pass them to existing code without breaking anything.

    You can iterate over the same collection in parallel because each iterator object contains its own iteration state.

    For the same reason, you can delay an iteration and continue it when needed.

Cons:
    Applying the pattern can be an overkill if your app only works with simple collections.

    Performence: Using an iterator may be less efficient than going through elements of some specialized collections directly.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <vector>
#include <memory>
#include <ios>
#include <algorithm>

/**
 * Iterator Design Pattern
 *
 * Intent: Lets you traverse elements of a collection without exposing its
 * underlying representation (list, stack, tree, etc.).
 */

/**
 * C++ has its own implementation of iterator that works with a different
 * generics containers defined by the standard library.
 */

template <typename T, typename U>
class Iterator
{
public:
    typedef typename std::vector<T>::iterator iter_type;
    // Iterator(U *p_data, bool reverse = false) : m_p_data_(p_data)
    Iterator(U *p_data) : m_p_data_(p_data)
    {
        m_it_ = m_p_data_->m_data_.begin();
    }

    void First()
    {
        m_it_ = m_p_data_->m_data_.begin();
    }

    void Next()
    {
        m_it_++;
    }

    bool IsDone()
    {
        return (m_it_ == m_p_data_->m_data_.end());
    }

    iter_type Current()
    {
        return m_it_;
    }

private:
    U *m_p_data_;
    iter_type m_it_{};
};

/**
 * Generic Collections/Containers provides one or several methods for retrieving
 * fresh iterator instances, compatible with the collection class.
 */

template <class T>
class Container
{
    friend class Iterator<T, Container>;

public:
    void Add(T a)
    {
        m_data_.push_back(a);
    }

    std::shared_ptr<Iterator<T, Container>> CreateIterator()
    {
        return std::make_shared<Iterator<T, Container>>(this);
    }

private:
    std::vector<T> m_data_{};
};

class Data
{
public:
    Data(int a = 0) : m_data_(a) {}

    void set_data(int a)
    {
        m_data_ = a;
    }

    int data()
    {
        return m_data_;
    }

private:
    int m_data_;
};

/**
 * The client code may or may not know about the Concrete Iterator or Collection
 * classes, for this implementation the container is generic so you can used
 * with an int or with a custom class.
 */
void ClientCode()
{
    std::cout << "________________Iterator with int______________________________________" << std::endl;
    Container<int> cont;

    for (int i = 0; i < 10; i++)
    {
        cont.Add(i);
    }

    auto it = cont.CreateIterator();
    for (it->First(); !it->IsDone(); it->Next())
    {
        std::cout << *it->Current() << std::endl;
    }

    Container<Data> cont2;
    Data a(100), b(1000), c(10000);
    cont2.Add(a);
    cont2.Add(b);
    cont2.Add(c);

    std::cout << "________________Iterator with custom Class______________________________" << std::endl;
    auto it2 = cont2.CreateIterator();
    for (it2->First(); !it2->IsDone(); it2->Next())
    {
        std::cout << it2->Current()->data() << std::endl;
    }
}

int main()
{
    std::cout << std::endl
              << "Iterator 1" << std::endl
              << std::endl;

    ClientCode();
    std::cout
        << std::endl
        << "Iterator End" << std::endl
        << std::endl;

    return 0;
}
