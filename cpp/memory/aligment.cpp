#include <iostream>
#include <cassert>
#include <thread>
#include <atomic>


// Alignment = placing data at addresses divisible by its alignment requirement.
// To allow CPU to load data using fast hardware paths without extra shifting/merging.
// Rule: address % alignof(T) == 0
struct A
{
    char a; // 1 byte
    int b;  // 4 bytes
};

// | a | pad | pad | pad | b b b b |

// Disabling padding with #pragma pack
#pragma pack(push, 1)
struct B {
    char a;
    int b;
};
#pragma pack(pop)

// Disabling padding with __attribute__((packed)) 
struct __attribute__((packed)) C {
    char a;
    int b;
};


// Memory → Cache (64B) → MUX → Register → ALU → Register → Memory

/*
int a = 10;
int b = 20;
int c = a + b;

mov eax, [a]
mov ebx, [b]
add eax, ebx
mov [c], eax



ALU (Arithmetic Logic Unit)
+ - * / AND OR XOR comparisons

!!! Multiplexer (MUX) !!!

Hardware selector: chooses one input out of many
inputs: b0 b1 b2 ... b63
select index → output = b[index]


!!! How CPU reads memory (important) !!!

char(1B)
select block → select byte

int(4B, alignment 4)
select block → select 4 bytes

int (unaligned)
select 2 blocks → select 4 bytes → shift and mask to get the correct data
slower than aligned access due to extra steps (selecting multiple blocks, shifting, masking)
*/


// !!! False sharing vs True sharing !!!

// Cache line is the smallest unit of data transferred between the CPU and cache
// Typical size = 64 bytes

/*
False sharing
    Threads modify different variables in the same cache line
*/

struct Data {
    int a;
    int b;
};
// both a and b in same cache line → slowdown

Data d;

void w1() { for(int i=0;i<1e7;i++) d.a++; }
void w2() { for(int i=0;i<1e7;i++) d.b++; }



// True sharing
/*
Threads access the same memory location or logically the same data.
*/

std::atomic<int> counter{0};

void wt1() {
    for (int i = 0; i < 1'000'000; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

void wt2() {
    for (int i = 0; i < 1'000'000; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}



/*
std::hardware_destructive_interference_size
avoid false sharing

Ensures that variable is on a separate cache line from other variables, preventing false sharing
Minimum offset between two objects to avoid false sharing.
Guaranteed to be at least alignof(std::max_align_t).
*/
struct keep_apart // TwoCacheLiner
{
    alignas(std::hardware_destructive_interference_size) std::atomic<int> cat;
    alignas(std::hardware_destructive_interference_size) std::atomic<int> dog;
};


/*
std::hardware_constructive_interference_size

Ensures that related data is on the same cache line, promoting true sharing and better cache utilization.

Maximum size of contiguous memory to promote true sharing. Guaranteed to be at least alignof(std::max_align_t)
*/

struct alignas(std::hardware_constructive_interference_size)
OneCacheLiner // occupies one cache line
{
    std::atomic_uint64_t x{};
    std::atomic_uint64_t y{};
};


// !!! Empty Base Optimization (EBO) !!!

struct Empty {};

struct Derived : Empty {
    int a;
};

// Tail Padding reuse

struct E {
    char a;
    // padding here
};

struct D : E {
    char b; // may use padding
    int c;
};

struct F {
    char b;
    int c;
};



// !!! Bit fields !!!

struct Flags {
    unsigned ready : 1;
    unsigned error : 1;
    unsigned mode  : 2;
};

struct PacketHeader {
    unsigned version : 4;
    unsigned type    : 4;
    unsigned length  : 8;
};

struct Flags_char {
    char ready : 1;
    char error : 1;
    char mode  : 2;
};

/*
Important warnings

Bit-fields are tricky in interviews because layout details are often implementation-defined.

That means:

exact ordering of bits may depend on compiler/ABI
packing may vary
portability can be poor for binary protocols

So for network/file formats, people often prefer manual masks/shifts.

Common limitations

You usually cannot:

take the address of a bit-field
make atomic assumptions about independent bit-fields
rely on exact bit ordering portably
*/




int main()
{
    std::cout << "Size of A: " << sizeof(A) << std::endl;       // Output: 8
    std::cout << "Alignment of A: " << alignof(A) << std::endl; // Output: 4
    std::cout << "Size of string: " << sizeof(std::string) << std::endl;       // Output: 32 (implementation-defined)
    std::cout << "Alignment of string: " << alignof(std::string) << std::endl; // Output: 8 (implementation-defined)
    std::cout << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "Pragma pack(push, 1) disables padding:" << std::endl;
    std::cout << "Size of B: " << sizeof(B) << std::endl;       // Output: 5
    std::cout << "Alignment of B: " << alignof(B) << std::endl; // Output: 1
    std::cout << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "__attribute__((packed)) disables padding:" << std::endl;
    std::cout << "Size of C: " << sizeof(C) << std::endl;       // Output: 5
    std::cout << "Alignment of C: " << alignof(C) << std::endl; // Output: 1
    std::cout << std::endl;

    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "False sharing vs True sharing:" << std::endl;
    std::cout << "False sharing:" << std::endl;
    std::cout << "Threads modify different variables in the same cache line" << std::endl;
    std::thread t1(w1);
    std::thread t2(w2);
    t1.join();
    t2.join();
    std::thread t3(wt1);
    std::thread t4(wt2);
    t3.join();
    t4.join();
    std::cout << "True sharing:" << std::endl;
    std::cout << "Counter: " << counter.load() << std::endl;
    std::cout << "Threads access the same memory location or logically the same data" << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "std::hardware_destructive_interference_size:" << std::endl;
    std::cout << "Ensures that variable is on a separate cache line from other variables, preventing false sharing" << std::endl;
    std::cout << "Size of keep_apart: " << sizeof(keep_apart) << std::endl; // Output: 128 (2 atomic ints, each aligned to 64 bytes)
    std::cout << "Alignment of keep_apart: " << alignof(keep_apart) << std::endl; // Output: 64
    std::cout << "std::hardware_constructive_interference_size:" << std::endl;
    std::cout << "Ensures that related data is on the same cache line, promoting true sharing and better cache utilization" << std::endl;
    std::cout << "Size of OneCacheLiner: " << sizeof(OneCacheLiner) << std::endl; // Output: 64
    std::cout << "Alignment of OneCacheLiner: " << alignof(OneCacheLiner) << std::endl; // Output: 64
    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "Empty Base Optimization (EBO):" << std::endl;
    std::cout << "Size of Derived: " << sizeof(Derived) << std::endl; // Output: 4 (not 8, because Empty has no data members)
    std::cout << "Alignment of Derived: " << alignof(Derived) << std::endl; // Output: 4
    std::cout << "Size of Empty: " << sizeof(Empty) << std::endl; // Output: 1 (empty class still has size 1)
    std::cout << "Alignment of Empty: " << alignof(Empty) << std::endl; // Output: 1
    std::cout << "------------------------------------------------------" << std::endl;

    std::cout << "Padding reuse:" << std::endl;
    std::cout << "Size of D: " << sizeof(D) << std::endl; // Output: 2 (char a + char b, padding reused)
    std::cout << "Alignment of D: " << alignof(D) << std::endl; // Output: 1
    std::cout << "Size of E: " << sizeof(E) << std::endl; // Output: 4 (char a + 3 bytes padding)
    std::cout << "Alignment of E: " << alignof(E) << std::endl; // Output: 4
    std::cout << "Size of F: " << sizeof(F) << std::endl; // Output: 4 (char b + 3 bytes padding)
    std::cout << "Alignment of F: " << alignof(F) << std::endl; // Output: 4
    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "Bit fields:" << std::endl;
    std::cout << "Size of Flags: " << sizeof(Flags) << std::endl; // Output: 4 (1 byte for ready+error+mode, plus padding to align to 4 bytes)
    std::cout << "Alignment of Flags: " << alignof(Flags) << std::endl; // Output: 4
    std::cout << "Size of PacketHeader: " << sizeof(PacketHeader) << std::endl; // Output: 4 (1 byte for version+type+length, plus padding to align to 4 bytes)
    std::cout << "Alignment of PacketHeader: " << alignof(PacketHeader) << std::endl; // Output: 4
    std::cout << "Size of Flags_char: " << sizeof(Flags_char) << std::endl; // Output: 1 (all bit-fields fit in 1 byte, no padding needed)
    std::cout << "Alignment of Flags_char: " << alignof(Flags_char) << std::endl; // Output: 1  


    std::cout << std::endl;

    return 0;
}