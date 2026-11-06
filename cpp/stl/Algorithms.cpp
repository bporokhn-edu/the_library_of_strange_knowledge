#include <algorithm>
#include <iostream>
#include <vector>
#include <execution>
#include <iterator>
#include <numeric>
#include <functional>
#include <array>
#include <complex>
#include <cassert>
#include <string_view>
#include <iomanip>
#include <thread>
#include <chrono>
#include <list>

#include <execution>
#define SEQ std::execution::seq,
#define PAR std::execution::par,

template <class Container, class Size, class T>
constexpr bool consecutive_values(const Container &c, Size count, const T &v)
{
    return std::search_n(std::begin(c), std::end(c), count, v) != std::end(c);
}
int main()
{

    std::cout << "Batch operations" << std::endl;
    std::cout << "for_each" << std::endl; // iterate over (first, end] and performe action
    {
        std::vector<int> v{3, -4, 2, -8, 15, 267};

        auto print = [](const int &n)
        { std::cout << n << ' '; };

        std::cout << "before:\t";
        std::for_each(v.cbegin(), v.cend(), print);
        std::cout << '\n';

        // increment elements in-place
        std::for_each(std::execution::par, v.begin(), v.end(), [](int &n)
                      { n++; });

        std::cout << "after:\t";
        std::for_each(v.cbegin(), v.cend(), print);
        std::cout << '\n';

        struct Sum
        {
            void operator()(int n) { sum += n; }
            int sum{0};
        };

        // invoke Sum::operator() for each element
        Sum s = std::for_each(v.cbegin(), v.cend(), Sum());
        std::cout << "sum:\t" << s.sum << '\n';
    }
    std::cout << std::endl;

    std::cout << "for_each_n" << std::endl; // iterate over (first, first + n] and performe action
    {
        std::vector<int> v{31, -4, 2, -82, 15, 267};

        auto print = [](const int &n)
        { std::cout << n << ' '; };

        std::cout << "before:\t";
        std::for_each_n(v.cbegin(), v.size(), print);
        std::cout << '\n';

        // increment elements in-place
        // std::for_each_n(std::execution::par, v.begin(), 100, [](int &n) // can run without error,
        //                                                    but on another vector creatin can get malloc: corrupted top size

        std::for_each_n(std::execution::par, v.begin(), v.size(), [](int &n)
                        { n++; });

        std::cout << "after:\t";
        std::for_each_n(v.cbegin(), v.size(), print);
        std::cout << '\n';

        struct Sum
        {
            void operator()(int n) { sum += n; }
            int sum{0};
        };

        // invoke Sum::operator() for each element
        Sum s;
        std::for_each_n(v.cbegin(), v.size(), std::ref(s));
        std::cout << "sum:\t" << s.sum << '\n';
    }
    std::cout << std::endl;
    std::cout << "Search operations" << std::endl;

    std::cout << "all/any/none _of" << std::endl; // check preidcate over all elemnts
    {
        std::vector<int> v1(10, 2);
        std::cout << "Among the numbers: ";
        std::partial_sum(v1.cbegin(), v1.cend(), v1.begin());
        std::copy(v1.cbegin(), v1.cend(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << '\n';

        if (std::all_of(v1.cbegin(), v1.cend(), [](int i)
                        { return i % 2 == 0; }))
            std::cout << "All numbers are even\n";

        if (std::none_of(v1.cbegin(), v1.cend(), std::bind(std::modulus<>(), std::placeholders::_1, 2)))
            std::cout << "None of them are odd\n";

        struct DivisibleBy
        {
            const int d;
            DivisibleBy(int n) : d(n) {}
            bool operator()(int n) const { return n % d == 0; }
        };

        if (std::any_of(v1.cbegin(), v1.cend(), DivisibleBy(7)))
            std::cout << "At least one number is divisible by 7\n";
    }
    std::cout << std::endl;
    std::cout << "find if/if_not" << std::endl;
    {
        const auto v = {1, 2, 3, 4};

        for (const int n : {3, 5})
            (std::find(v.begin(), v.end(), n) == std::end(v))
                ? std::cout << "v does not contain " << n << '\n'
                : std::cout << "v contains " << n << '\n';

        auto is_even = [](int i)
        { return i % 2 == 0; };

        for (const auto &w : {std::array{3, 1, 4}, {1, 3, 5}})
            if (auto it = std::find_if(std::begin(w), std::end(w), is_even); it != std::end(w))
                std::cout << "w contains an even number " << *it << '\n';
            else
                std::cout << "w does not contain even numbers\n";

        std::vector<std::complex<double>> nums{{5, 2}, {4, 2}, {3, 1}};

        const auto it = std::find_if_not(nums.begin(), nums.end(), [](const auto &c)
                                         { return c == std::complex<double>{4, 2}; });
        assert(it == nums.begin());
    }
    std::cout << std::endl;
    std::cout << "find_end" << std::endl; // find the last sub-sequence in the sequence (can use comperator)

    {
        const auto v = {1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4};
        auto print_result = [](auto result, const auto &v)
        {
            result == v.end()
                ? std::cout << "Sequence not found\n"
                : std::cout << "Last occurrence is at: " << std::distance(v.begin(), result)
                            << '\n';
        };
        for (auto const &x : {std::array{1, 2, 3}, {4, 5, 6}})
        {
            auto iter = std::find_end(v.begin(), v.end(), x.begin(), x.end()); // overload (1)
            print_result(iter, v);
        }

        for (auto const &x : {std::array{-1, -2, -3}, {-4, -5, -6}})
        {
            auto iter = std::find_end(v.begin(), v.end(), x.begin(), x.end(), // overload (3)
                                      [](int x, int y)
                                      {
                                          return std::abs(x) == std::abs(y);
                                      });
            print_result(iter, v);
        }
    }
    std::cout << std::endl;
    std::cout << "find_first_of" << std::endl;
    {
        auto print_sequence = [](const auto id, const auto &seq, int pos = -1)
        {
            std::cout << id << "{ ";
            for (int i = 0; auto const &e : seq)
            {
                const bool mark{i == pos};
                std::cout << (i++ ? ", " : "");
                std::cout << (mark ? "[ " : "") << e << (mark ? " ]" : "");
            }
            std::cout << " }\n";
        };
        const std::vector<int> v{0, 2, 3, 25, 5};
        const auto t1 = {19, 10, 3, 4};
        const auto t2 = {1, 6, 7, 9};

        auto find_any_of = [print_sequence](const auto &v, const auto &t)
        {
            const auto result = std::find_first_of(v.begin(), v.end(),
                                                   t.begin(), t.end());
            if (result == v.end())
            {
                std::cout << "No elements of v are equal to any element of ";
                print_sequence("t = ", t);
                print_sequence("v = ", v);
            }
            else
            {
                const auto pos = std::distance(v.begin(), result);
                std::cout << "Found a match (" << *result << ") at position " << pos;
                print_sequence(", where t = ", t);
                print_sequence("v = ", v, pos);
            }
        };

        find_any_of(v, t1);
        find_any_of(v, t2);
    }
    std::cout << std::endl;

    std::cout << "adjacent_find" << std::endl; // check if have two equel element in vector or use predicate for each neighbor elements
    {
        std::vector<int> v1{0, 1, 2, 3, 40, 40, 41, 41, 5};

        auto i1 = std::adjacent_find(v1.begin(), v1.end());

        if (i1 == v1.end())
            std::cout << "No matching adjacent elements\n";
        else
            std::cout << "The first adjacent pair of equal elements is at "
                      << std::distance(v1.begin(), i1) << ", *i1 = "
                      << *i1 << '\n';

        auto i2 = std::adjacent_find(v1.begin(), v1.end(), std::greater<int>());
        if (i2 == v1.end())
            std::cout << "The entire vector is sorted in ascending order\n";
        else
            std::cout << "The last element in the non-decreasing subsequence is at "
                      << std::distance(v1.begin(), i2) << ", *i2 = " << *i2 << '\n';
    }
    std::cout << std::endl;
    std::cout << "count/count_if" << std::endl; // count how many "value"/(predicate returns true) in array
    {
        constexpr std::array v{1, 2, 3, 4, 4, 3, 7, 8, 9, 10};
        std::cout << "v: ";
        std::copy(v.cbegin(), v.cend(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << '\n';

        // Determine how many integers match a target value.
        for (const int target : {3, 4, 5})
        {
            const int num_items = std::count(v.cbegin(), v.cend(), target);
            std::cout << "number: " << target << ", count: " << num_items << '\n';
        }

        // Use a lambda expression to count elements divisible by 4.
        int count_div4 = std::count_if(v.begin(), v.end(), [](int i)
                                       { return i % 4 == 0; });
        std::cout << "numbers divisible by four: " << count_div4 << '\n';

        // A simplified version of `distance` with O(N) complexity:
        auto distance = [](auto first, auto last)
        {
            return std::count_if(first, last, [](auto)
                                 { return true; });
        };
        static_assert(distance(v.begin(), v.end()) == 10);

        std::array<std::complex<double>, 3> nums{{{4, 2}, {1, 3}, {4, 2}}};
        auto c = std::count(nums.cbegin(), nums.cend(), std::complex<double>{4, 2});
        assert(c == 2);
    }
    std::cout << std::endl;
    std::cout << "mismach" << std::endl; // finds the first position where two ranges differ
    {
        auto mirror_ends = [](const std::string &in)
        {
            return std::string(in.begin(),
                               std::mismatch(in.begin(), in.end(), in.rbegin()).first);
        };

        std::cout << mirror_ends("abXYZba") << '\n'
                  << mirror_ends("abca") << '\n'
                  << mirror_ends("aba") << '\n';

        std::vector<int> vec1 = {1, 2, 3, 4, 5};
        std::vector<int> vec2 = {1, 2, 3};
        auto print_res = [&](const auto &result)
        {
            if (result.first == vec1.end() || result.second == vec2.end())
            {
                std::cout << "No mismatch found.\n";
            }
            else
            {
                std::cout << "First mismatch at position: "
                          << std::distance(vec1.begin(), result.first) << "\n";
                std::cout << "vec1 value: " << *result.first << ", vec2 value: " << (result.second != vec2.end() ? std::to_string(*result.second) : "end") << "\n";
            }
        };

        // Find the first mismatch
        auto result = std::mismatch(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        print_res(result);
        vec1 = {1, 2, 3};
        vec2 = {1, 2, 2, 4, 5};
        result = std::mismatch(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        print_res(result);
        std::cout << "vectors are " << (vec1 == vec2 ? "equal" : "not equal") << std::endl;
    }

    std::cout << std::endl;
    std::cout << "equal" << std::endl; // finds the first position where two ranges differ
    {
        auto is_palindrome = [](const std::string_view &s)
        {
            return std::equal(s.cbegin(), s.cbegin() + s.size() / 2, s.crbegin());
        };

        auto test = [&](const std::string_view &s)
        {
            std::cout << std::quoted(s)
                      << (is_palindrome(s) ? " is" : " is not")
                      << " a palindrome\n";
        };

        test("radar");
        test("hello");
    }
    std::cout << std::endl;
    std::cout << "search" << std::endl;
    using namespace std::literals;

    {
        auto contains = [](const auto &cont, std::string_view s)
        {
            // str.find() (or str.contains(), since C++23) can be used as well
            return std::search(cont.begin(), cont.end(), s.begin(), s.end()) != cont.end();
        };
        const auto str{"why waste time learning, when ignorance is instantaneous?"sv};
        assert(contains(str, "learning"));
        assert(not contains(str, "lemming"));

        const std::vector vec(str.begin(), str.end());
        assert(contains(vec, "learning"));
        assert(not contains(vec, "leaning"));

        // The C++17 overload with searchers demo:
        constexpr auto quote{
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed "
            "do eiusmod tempor incididunt ut labore et dolore magna aliqua"sv};

        for (const auto word : {"pisci"sv, "Pisci"sv})
        {
            std::cout << "The string " << std::quoted(word) << ' ';
            // const std::boyer_moore_searcher searcher(word.begin(), word.end());
            const std::boyer_moore_horspool_searcher searcher(word.begin(), word.end());

            const auto it = std::search(quote.begin(), quote.end(), searcher);
            if (it == quote.end())
                std::cout << "not found\n";
            else
                std::cout << "found at offset " << std::distance(quote.begin(), it) << '\n';
        }
    }

    std::cout << std::endl;
    std::cout << "search_n" << std::endl;
    // Searches the range [first, last) for the first sequence of count identical elements, each equal to the given value.
    {
        constexpr char sequence[] = ".0_0.000.0_0.";

        bool res = std::search_n(std::begin(sequence), std::end(sequence), 3, '0') != std::end(sequence);
        assert(res);

        for (int n : {4, 3, 2})
            std::cout << std::boolalpha
                      << "Has " << n << " consecutive zeros: "
                      << (std::search_n(std::begin(sequence), std::end(sequence), n, '0') != std::end(sequence)) << '\n';

        std::vector<std::complex<double>> nums{{4, 2}, {4, 2}, {1, 3}};
        auto it = std::search_n(nums.cbegin(), nums.cend(), 2, std::complex<double>{4, 2});
        assert(it == nums.begin());
    }
    std::cout << std::endl;

    std::cout << "Modifying sequence operations" << std::endl;
    std::cout << "Copy operations" << std::endl;

    std::cout << "copy/copy_if" << std::endl;
    {
        std::vector<int> from_vector(10);
        std::iota(from_vector.begin(), from_vector.end(), 0);

        std::vector<int> to_vector(3, 4);
        std::copy(from_vector.begin(), from_vector.end(),
                  std::back_inserter(to_vector));
        // or, alternatively,
        //  std::vector<int> to_vector(from_vector.size());
        //  std::copy(from_vector.begin(), from_vector.end(), to_vector.begin());
        // either way is equivalent to
        //  std::vector<int> to_vector = from_vector;

        std::cout << "to_vector contains: ";

        std::copy(to_vector.begin(), to_vector.end(),
                  std::ostream_iterator<int>(std::cout, " "));
        std::cout << '\n';

        std::cout << "odd numbers in to_vector are: ";

        std::copy_if(to_vector.begin(), to_vector.end(),
                     std::ostream_iterator<int>(std::cout, " "),
                     [](int x)
                     { return x % 2 != 0; });
        std::cout << '\n';

        std::cout << "to_vector contains these multiples of 3: ";

        to_vector.clear();
        std::copy_if(from_vector.begin(), from_vector.end(),
                     std::back_inserter(to_vector),
                     [](int x)
                     { return x % 3 == 0; });

        for (const int x : to_vector)
            std::cout << x << ' ';
        std::cout << '\n';
    }
    std::cout << std::endl;
    std::cout << "copy_n" << std::endl;
    {
        std::string in{"1234567890"};
        std::string out;

        std::copy_n(in.begin(), 4, std::back_inserter(out));
        std::cout << out << '\n';

        std::vector<int> v_in(128);
        std::iota(v_in.begin(), v_in.end(), 1);
        std::vector<int> v_out(v_in.size());

        std::copy_n(v_in.cbegin(), 100, v_out.begin());
        std::cout << std::accumulate(v_out.begin(), v_out.end(), 0) << '\n';
    }
    std::cout << std::endl;
    std::cout << "copy_backward" << std::endl;
    {
        std::vector<int> source(4);
        std::iota(source.begin(), source.end(), 1); // fills with 1, 2, 3, 4

        std::vector<int> destination(6);

        std::copy_backward(source.begin(), source.end(), destination.end());

        std::cout << "destination contains: ";
        for (auto i : destination)
            std::cout << i << ' ';
        std::cout << '\n';
    }
    std::cout << std::endl;
    std::cout << "move" << std::endl;
    {
        auto f = [](int n)
        {
            std::this_thread::sleep_for(std::chrono::seconds(n));
            std::cout << "thread " << n << " ended" << std::endl;
        };
        std::vector<std::jthread> v;
        v.emplace_back(f, 1);
        v.emplace_back(f, 2);
        v.emplace_back(f, 3);
        std::list<std::jthread> l;

        // copy() would not compile, because std::jthread is noncopyable
        std::move(v.begin(), v.end(), std::back_inserter(l));
    }
    std::cout << std::endl;
    std::cout << "move_backward" << std::endl;
    {
        using container = std::vector<std::string>;

        auto print = [](std::string_view comment, const container &src, const container &dst = {})
        {
            auto prn = [](std::string_view name, const container &cont)
            {
                std::cout << name;
                for (const auto &s : cont)
                    std::cout << (s.empty() ? "∙" : s.data()) << ' ';
                std::cout << '\n';
            };
            std::cout << comment << '\n';
            prn("src: ", src);
            if (dst.empty())
                return;
            prn("dst: ", dst);
        };

        container src{"foo", "bar", "baz"};
        container dst{"qux", "quux", "quuz", "corge"};
        print("Non-overlapping case; before move_backward:", src, dst);
        std::move_backward(src.begin(), src.end(), dst.end());
        print("After:", src, dst);

        src = {"snap", "crackle", "pop", "lock", "drop"};
        print("Overlapping case; before move_backward:", src);
        std::move_backward(src.begin(), std::next(src.begin(), 3), src.end());
        print("After:", src);
    }
    std::cout << std::endl;
    std::cout << "swap_ranges" << std::endl;
    {
        auto print = [](auto comment, auto const &seq)
        {
            std::cout << comment;
            for (const auto &e : seq)
                std::cout << e << ' ';
            std::cout << '\n';
        };
        std::vector<char> v{'a', 'b', 'c', 'd', 'e'};
        std::list<char> l{'1', '2', '3', '4', '5'};

        print("Before swap_ranges:\n"
              "v: ",
              v);
        print("l: ", l);

        std::swap_ranges(v.begin(), v.begin() + 3, l.begin());

        print("After swap_ranges:\n"
              "v: ",
              v);
        print("l: ", l);
    }

    std::cout << std::endl;
    std::cout << "Transform" << std::endl;
    {
        auto print_ordinals = [](const std::vector<unsigned> &ordinals)
        {
            std::cout << "ordinals: ";
            for (unsigned ord : ordinals)
                std::cout << std::setw(3) << ord << ' ';
            std::cout << '\n';
        };

        auto to_uppercase = [](unsigned char c)
        {
            return std::toupper(c);
        };

        auto to_uppercase_inplace = [&](char &c)
        {
            c = to_uppercase(c);
        };
        auto unary_transform_example = [&](std::string &hello, std::string world)
        {
            // Transform string to uppercase in-place

            std::transform(hello.cbegin(), hello.cend(), hello.begin(), to_uppercase);
            std::cout << "hello = " << std::quoted(hello) << '\n';

            // for_each version (see Notes above)
            std::for_each(world.begin(), world.end(), to_uppercase_inplace);
            std::cout << "world = " << std::quoted(world) << '\n';
        };

        auto binary_transform_example = [&](std::vector<unsigned> ordinals)
        {
            // Transform numbers to doubled values

            print_ordinals(ordinals);

            std::transform(ordinals.cbegin(), ordinals.cend(), ordinals.cbegin(),
                           ordinals.begin(), std::plus<>{});

            print_ordinals(ordinals);
        };

        std::string hello("hello");
        unary_transform_example(hello, "world");

        std::vector<unsigned> ordinals;
        std::copy(hello.cbegin(), hello.cend(), std::back_inserter(ordinals));
        binary_transform_example(std::move(ordinals));
    }
    std::cout << std::endl;
    std::cout << "replace" << std::endl;
    {
        auto println = [](const auto &seq)
        {
            for (const auto &e : seq)
                std::cout << e << ' ';
            std::cout << '\n';
        };
        std::array<int, 10> s{5, 7, 4, 2, 8, 6, 1, 9, 0, 3};

        // Replace all occurrences of 8 with 88.
        std::replace(s.begin(), s.end(), 8, 88);
        println(s);

        // Replace all values less than 5 with 55.
        std::replace_if(s.begin(), s.end(),
                        std::bind(std::less<int>(), std::placeholders::_1, 5), 55);
        println(s);

        std::array<std::complex<double>, 2> nums{{{1, 3}, {1, 3}}};
        std::replace(nums.begin(), nums.end(), std::complex<double>{1, 3},
                     std::complex<double>{4, 2});
        println(nums);
    }
    std::cout << std::endl;
    std::cout << "replace_copy_if" << std::endl;
    {
        auto println = [](const auto &seq)
        {
            for (const auto &e : seq)
                std::cout << e << ' ';
            std::cout << '\n';
        };
        std::vector<short> src{3, 1, 4, 1, 5, 9, 2, 6, 5};
        println(src);
        std::vector<int> dst(src.size());
        std::replace_copy_if(src.cbegin(), src.cend(), dst.begin(), [](short n)
                             { return n > 5; }, 0);
        println(dst);

        std::vector<std::complex<double>> src2{{1, 3}, {2, 4}, {3, 5}},
            dst2(src2.size());
        println(src2);
        std::replace_copy_if(src2.cbegin(), src2.cend(), dst2.begin(), [](std::complex<double> z)
                             { return std::abs(z) < 5; }, std::complex<double>{4, 2});
        println(dst2);
    }
    {
        std::cout << std::fixed << std::setprecision(1);

        auto eval = [](auto fun)
        {
            const auto t1 = std::chrono::high_resolution_clock::now();
            const auto [name, result] = fun();
            const auto t2 = std::chrono::high_resolution_clock::now();
            const std::chrono::duration<double, std::milli> ms = t2 - t1;
            std::cout << std::setw(28) << std::left << name << "sum: "
                      << result << '\t' << "time: " << ms.count() << " ms\n";
        };

        {
            const std::vector<double> v(100'000'007, 0.1);

            eval([&v]
                 { return std::pair{"std::accumulate (double)",
                                    std::accumulate(v.cbegin(), v.cend(), 0.0)}; });
            eval([&v]
                 { return std::pair{"std::reduce (seq, double)",
                                    std::reduce(SEQ v.cbegin(), v.cend())}; });
            eval([&v]
                 { return std::pair{"std::reduce (par, double)",
                                    std::reduce(PAR v.cbegin(), v.cend())}; });
        }

        {
            const std::vector<long> v(100'000'007, 1);

            eval([&v]
                 { return std::pair{"std::accumulate (long)",
                                    std::accumulate(v.cbegin(), v.cend(), 0l)}; });
            eval([&v]
                 { return std::pair{"std::reduce (seq, long)",
                                    std::reduce(SEQ v.cbegin(), v.cend())}; });
            eval([&v]
                 { return std::pair{"std::reduce (par, long)",
                                    std::reduce(PAR v.cbegin(), v.cend())}; });
        }
    }
}