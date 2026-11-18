/*
C++ VALUE CATEGORIES — study guide and runnable examples (C++17 and later)

Build from the library root:
  g++ -std=c++17 -Wall -Wextra -Wpedantic cpp/value_categories/ValueCategories.cpp -o /tmp/value_categories
  /tmp/value_categories

READ THIS FIRST
An expression has a type AND a value category. These are separate properties.
An object's lifetime and an expression's category are also different concepts.
For example, a variable declared int&& has reference type, but using its name
is an lvalue expression. const does not determine the value category.

1. THE FIVE NAMES (three primary categories, two overlapping groups)

                  expression
                 /          \
             glvalue        rvalue
             /    \        /    \
          lvalue   xvalue       prvalue

  lvalue:  identifies an object/function; examples: x, *ptr, a[i], "hello".
           A const object is still an lvalue when referred to by its name.
  xvalue:  identifies an object whose resources may be reused; std::move(x).
           Means "expiring value"; it does not itself end the object's lifetime.
  prvalue: computes a value or initializes an object; 42, x + 1, Widget{}.
           Means "pure rvalue". Since C++17 a class prvalue can initialize its
           destination directly; a temporary is materialized when needed.
  glvalue = lvalue OR xvalue: expressions that determine identity.
  rvalue  = prvalue OR xvalue: expressions usable as sources for moving.

The old "left/right side of assignment" explanation is incomplete:
  const int x = 1; // x is an lvalue, yet x = 2 is invalid.
  int y = x;      // an lvalue can appear on the right.
String literals are lvalue arrays, NOT prvalues. Most numeric literals are prvalues.

2. REFERENCES: BINDING (for int, without conversions)

Argument expression       int&        const int&       int&&
mutable lvalue            yes         yes              no
const lvalue              no          yes              no
mutable xvalue/prvalue    no          yes              yes
const xvalue              no          yes              no

const int&& can bind const rvalues, but cannot bind lvalues.
An int&& parameter is an rvalue reference; it is NOT a forwarding reference.

3. MOVE AND FORWARD
std::move(x) is a cast to an rvalue reference, yielding an xvalue for objects.
It does not move anything by itself. A selected constructor/assignment may move.
Moving a const object usually copies: a normal move constructor needs T&&,
while std::move(const_object) produces const T&&.
Moved-from standard-library objects are generally valid but unspecified unless
their contract says more. Do not assume a moved-from string is empty.
Our Trace class below explicitly leaves its source empty.

In template<class T> void f(T&&), where T is deduced, T&& is a forwarding
reference. For lvalue arguments T deduces as U&, so T&& collapses to U&.
For rvalue arguments T deduces as U, leaving U&&.
Reference collapsing: & + & -> &, & + && -> &, && + & -> &, && + && -> &&.
std::forward<T>(arg) preserves the caller's lvalue/rvalue distinction.
It does not preserve prvalue versus xvalue: forwarded rvalues are xvalues.
const T&& is not a forwarding reference. Neither is a class template's T&&
member parameter when that T is already fixed rather than deduced at the call.
auto&& usually follows forwarding deduction; braced initializer lists are special.

4. TYPE DEDUCTION
decltype(name) gives its declared type (special rule for unparenthesized names).
Otherwise decltype(expression) is T& for lvalues, T&& for xvalues, T for prvalues.
Use decltype((expression)) to examine the category even for a variable name.
auto usually drops references and top-level const and makes a value copy.
auto& binds an lvalue; auto&& deduces according to reference collapsing.
decltype(auto) follows decltype rules exactly, including parentheses!

5. LIFETIME AND RETURNS
const T& r = T{}; and T&& r = T{}; extend that temporary's lifetime locally.
Binding another reference to r does NOT extend it again.
A temporary bound to a reference parameter lasts to the end of the full
expression containing the call; returning that reference does not extend it.
Never return a reference to a local or a temporary created inside a function.
std::move does NOT extend lifetime.
Return objects by value: return T{}; has guaranteed elision since C++17.
return local; permits NRVO; without NRVO, an eligible local can be moved.
return std::move(local); prevents NRVO, so avoid it for a normal local return.

More cases to recognize (built-in operators unless stated otherwise):
  *p, ++x, (x = 3)       lvalue
  x++, x + 1, &x        prvalue
  std::move(x)          xvalue
  function returning T& lvalue call expression
  function returning T&& xvalue call expression
  function returning T  prvalue call expression (non-reference object T)
  object.member         lvalue when object is lvalue
  std::move(object).member  xvalue for a non-reference, non-static data member
Overloaded operators follow the return type of the selected function.
Bit-fields have additional reference-binding restrictions.

References for deeper reading (C++ working draft):
  https://eel.is/c++draft/basic.lval
  https://eel.is/c++draft/dcl.init.ref
  https://eel.is/c++draft/temp.deduct.call
  https://eel.is/c++draft/class.copy.elision
*/

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

// Pass decltype((expr)), not expr itself: a function parameter would turn a
// prvalue argument into a named lvalue, losing the original classification.
template <class T>
constexpr const char *category()
{
  if constexpr (std::is_lvalue_reference_v<T>)
    return "lvalue";
  else if constexpr (std::is_rvalue_reference_v<T>)
    return "xvalue";
  else
    return "prvalue";
}

#define SHOW(expr) std::cout << #expr << " -> " << category<decltype((expr))>() << '\n'

void select(int &) { std::cout << "int& (mutable lvalue)\n"; }
void select(const int &) { std::cout << "const int& (const lvalue or fallback)\n"; }
void select(int &&) { std::cout << "int&& (mutable rvalue)\n"; }

template <class T>
void relay(T &&argument)
{
  std::cout << "  named parameter: ";
  select(argument); // Always an lvalue expression here.
  std::cout << "  forwarded: ";
  select(std::forward<T>(argument)); // Restores lvalue/rvalue distinction.
}

struct Trace
{
  std::string text;
  explicit Trace(std::string value) : text(std::move(value)) {}
  Trace(const Trace &other) : text(other.text) { std::cout << "copy\n"; }
  Trace(Trace &&other) noexcept : text(std::move(other.text))
  {
    other.text.clear(); // Explicit guarantee for THIS demonstration class.
    std::cout << "move\n";
  }
};

Trace make_trace() { return Trace{"direct construction"}; }
int &reference_to(int &value) { return value; }
int &&rvalue_reference_to(int &value) { return std::move(value); }

struct RefQualified
{
  void inspect() & { std::cout << "inspect() &: lvalue receiver\n"; }
  void inspect() && { std::cout << "inspect() &&: rvalue receiver\n"; }
};

int main()
{
  std::cout << "1. Identify expressions (SHOW does not evaluate them)\n";
  int x = 10;
  const int cx = 20;
  int *p = &x;
  SHOW(x);                      // lvalue
  SHOW(cx);                     // lvalue
  SHOW(*p);                     // lvalue
  SHOW("hello");                // lvalue: const char[6]
  SHOW(42);                     // prvalue
  SHOW(x + 1);                  // prvalue
  SHOW(++x);                    // lvalue; x is NOT incremented by SHOW
  SHOW(x++);                    // prvalue; likewise unevaluated
  SHOW(std::move(x));           // xvalue
  SHOW(reference_to(x));        // lvalue
  SHOW(rvalue_reference_to(x)); // xvalue; refers to x, which remains alive
  SHOW(make_trace());           // prvalue; no Trace constructed by SHOW
  assert(x == 10);

  static_assert(std::is_same_v<decltype(x), int>);
  static_assert(std::is_same_v<decltype((x)), int &>);
  static_assert(std::is_same_v<decltype(std::move(x)), int &&>);
  static_assert(std::is_same_v<decltype(42), int>);
  static_assert(std::is_same_v<decltype(("hello")), const char (&)[6]>);

  std::cout << "\n2. Binding and overload selection\n";
  int &alias = x;
  alias = 11;
  assert(x == 11);
  const int &extended = 42; // Temporary lives as long as this local reference.
  int &&rr = 50;            // Same lifetime extension, but mutable.
  rr = 51;
  select(x);             // int&
  select(cx);            // const int&
  select(42);            // int&&
  select(std::move(x));  // int&&; no mutation of x
  select(std::move(cx)); // const int&; int&& cannot bind const int
  select(rr);            // int&: a named rvalue reference is an lvalue!
  select(std::move(rr)); // int&&
  assert(x == 11 && extended == 42 && rr == 51);
  // int& bad = 42;        // ERROR: mutable lvalue reference to a temporary.
  // int&& bad = x;        // ERROR: rvalue reference to an lvalue.

  std::cout << "\n3. std::move versus actually moving\n";
  Trace original{"payload"};
  Trace &&permission = std::move(original); // Binds; nothing is moved yet.
  assert(original.text == "payload");
  Trace copied{permission};           // copy: permission is an lvalue.
  Trace moved{std::move(permission)}; // move: consumes original's text.
  assert(original.text.empty());
  assert(copied.text == "payload" && moved.text == "payload");
  original.text = "reused"; // Moved-from objects can be reused.
  const Trace constant{"const payload"};
  Trace from_const{std::move(constant)}; // copy: const Trace&& -> const Trace&.
  assert(from_const.text == constant.text);
  auto owner = std::make_unique<int>(7);
  // auto duplicate = owner;                // ERROR: unique_ptr cannot be copied.
  auto new_owner = std::move(owner); // Actual ownership transfer.
  assert(!owner && *new_owner == 7); // unique_ptr guarantees empty source.

  std::cout << "\n4. Perfect forwarding\n";
  std::cout << "relay(x):\n";
  relay(x); // T = int&; both calls select int&.
  std::cout << "relay(42):\n";
  relay(42); // T = int; second call selects int&&.
  std::cout << "relay(cx):\n";
  relay(cx); // T = const int&; both select const int&.
  std::cout << "relay(std::move(x)):\n";
  relay(std::move(x)); // T = int; second selects int&&.

  std::cout << "\n5. auto, decltype(auto), and collapsing\n";
  auto value = cx;           // int: copy, drops top-level const.
  auto &ref = x;             // int&
  auto &&forward_lvalue = x; // int& after collapsing.
  auto &&forward_rvalue = 7; // int&&; local temporary lifetime extension.
  decltype(auto) copy = x;   // int (unparenthesized name rule).
  decltype(auto) same = (x); // int& (lvalue expression).
  static_assert(std::is_same_v<decltype(value), int>);
  static_assert(std::is_same_v<decltype(ref), int &>);
  static_assert(std::is_same_v<decltype(forward_lvalue), int &>);
  static_assert(std::is_same_v<decltype(forward_rvalue), int &&>);
  static_assert(std::is_same_v<decltype(copy), int>);
  static_assert(std::is_same_v<decltype(same), int &>);
  same = 99;
  assert(x == 99 && copy == 11 && value == 20 && forward_rvalue == 7);

  std::cout << "\n6. Members, ref-qualified methods, and copy elision\n";
  Trace object{"member"};
  SHOW(object.text);            // lvalue
  SHOW(std::move(object).text); // xvalue
  RefQualified receiver;
  receiver.inspect();          // & overload
  RefQualified{}.inspect();    // && overload
  Trace direct = make_trace(); // No copy/move: guaranteed C++17 elision.
  assert(direct.text == "direct construction");

  /* Do not uncomment these dangling-reference examples and then use them:
     auto bad_return() -> const std::string& { return std::string{"gone"}; }
     auto bad_local() -> int& { int n = 1; return n; }
     auto bad_decltype() -> decltype(auto) { int n = 1; return (n); }
     The last returns int& because of parentheses; it dangles too.

     Another trap:
       const std::string& identity(const std::string& s) { return s; }
       const auto& bad = identity(std::string{"gone"});
     The temporary dies at the declaration's semicolon, leaving bad dangling.
     These function definitions are illustrations, not valid nested functions.
  */

  std::cout << "\nAll checks passed. Try the exercises in the source.\n";
}

/*
EXERCISES — predict first, then verify with SHOW or static_assert
1. Classify: x, (x), std::move(x), std::move(cx), "abc", *p, &x, x++.
2. Why does select(rr) choose int& although rr was declared int&&?
3. In relay(42), replace forward<T> with move: works for this call. Now try
   relay(x). Why does unconditional move change the caller's intended category?
4. Add select(const int&&). Which overload does select(std::move(cx)) choose now?
5. Remove Trace's move constructor entirely (do not declare it deleted).
   Which constructor can now accept an xvalue? Contrast with an explicitly
   deleted move constructor: it can win overload resolution and cause an error.
6. Why is returning (local) dangerous with decltype(auto), but returning local
   gives a value for a non-reference local variable?

ANSWERS
1. lvalue, lvalue, xvalue, xvalue, lvalue, lvalue, prvalue, prvalue.
2. A named reference is an lvalue expression, independently of its declared type.
3. move casts even the lvalue caller's object to an xvalue; forward respects T.
4. const int&&, which is a better binding here than const int&.
5. The const Trace& copy constructor can bind rvalues. Moving is not guaranteed
   merely because an expression is an rvalue. Update the moved-from assertions
   when experimenting: a copy leaves the source text unchanged.
6. decltype((local)) is T&; decltype(local) is the local's declared type T.
*/
