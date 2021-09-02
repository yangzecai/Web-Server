#include <cassert>
#include <chrono>
#include <iostream>
#include <new>
#include <type_traits>

class Event {
public:
    Event(const char* file, const char* func, const char* level,
          uint32_t threadId)
        : file(file)
        , func(func)
        , level(level)
        , threadId(threadId)
    {
    }

    const char* file;
    const char* func;
    const char* level;
    uint32_t threadId;
};

int main()
{
    Event* data = nullptr;
    static std::aligned_storage<sizeof(Event), alignof(Event)>::type dataMem;

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        Event* a = new Event(__FILE__, __FUNCTION__, "INFO", 32);
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        data = new (&dataMem) Event(__FILE__, __FUNCTION__, "INFO", 32);
    }
    auto t3 = std::chrono::high_resolution_clock::now();

    std::cout
        << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()
        << std::endl;
    std::cout
        << std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count()
        << std::endl;
}

// #include <iostream>
// #include <type_traits>
// #include <string>

// class A {
// public:
//     A() {}
//     ~A() {}
//     A(const A&) = default;

// private:
//     int a_;
// };

// template<std::size_t N>
// class static_vector
// {
//     // N 个 T 的正确对齐的未初始化存储
//     typename std::aligned_storage<sizeof(A), alignof(A)>::type data[N];
//     std::size_t m_size = 0;

// public:
//     // 于对齐存储创建对象
//     template<typename ...Args> void emplace_back(Args&&... args)
//     {
//         if( m_size >= N ) // 可行的错误处理
//             throw std::bad_alloc{};
//         new(data+m_size) A(std::forward<Args>(args)...);
//         ++m_size;
//     }

//     // 访问对齐存储中的对象
//     const A& operator[](std::size_t pos) const
//     {
//         // 注意： C++17 起需要 std::launder
//         return *reinterpret_cast<const A*>(data+pos);
//     }

//     // 从对齐存储删除对象
//     ~static_vector()
//     {
//         for(std::size_t pos = 0; pos < m_size; ++pos) {
//             // 注意： C++17 起需要 std::launder
//             reinterpret_cast<A*>(data+pos)->A::~A();
//         }
//     }
// };

// int main()
// {
//     static_vector<10> v1;
//     v1.emplace_back(5);
//     v1.emplace_back(10);
//     // std::cout << v1[0] << '\n' << v1[1] << '\n';
// }