#include <iostream>
#include <thread>

#include <cassert>

template <class T, uint32_t CAP>
class SPSCQueue {
public:
    static_assert(CAP && !(CAP & (CAP - 1))); // CAP不为0且是2的幂次

    T* alloc()
    {
        if (producerPos_ - consumerCachePos_ == CAP) {
            if (consumerCachePos_ == consumerPos_) {
                return nullptr;
            }
            consumerCachePos_ = consumerPos_;
        }
        return &queue_[producerPos_ % CAP];
    }

    void push() { ++producerPos_; }

    T* front()
    {
        if (producerCachePos_ == consumerPos_) {
            if (producerCachePos_ == producerPos_) {
                return nullptr;
            }
            producerCachePos_ = producerPos_;
        }
        return &queue_[consumerPos_ % CAP];
    }

    void pop() { ++consumerPos_; }

private:
    T queue_[CAP];
    uint32_t producerPos_ = 0;
    uint32_t consumerPos_ = 0;

    uint32_t producerCachePos_ = 0;
    uint32_t consumerCachePos_ = 0;
};

// template <class T, uint32_t CNT>
// class SPSCQueue {
// public:
//     static_assert(CNT && !(CNT & (CNT - 1)), "CNT must be a power of 2");

//     T* alloc()
//     {
//         if (write_idx - read_idx_cach == CNT) { // 满了（负数？？？）
//             asm volatile("" : "=m"(read_idx) : :); // force read memory
//             read_idx_cach = read_idx;
//             if (__builtin_expect(write_idx - read_idx_cach == CNT, 0)) { // no
//                 enough space return nullptr;
//             }
//         }
//         return &data[write_idx % CNT];
//     }

//     void push()
//     {
//         asm volatile("" : : "m"(data), "m"(write_idx) :); // memory fence
//         ++write_idx;
//         asm volatile("" : : "m"(write_idx) :); // force write memory
//     }

//     template <typename Writer>
//     bool tryPush(Writer writer)
//     {
//         T* p = alloc();
//         if (!p)
//             return false;
//         writer(p);
//         push();
//         return true;
//     }

//     template <typename Writer>
//     void blockPush(Writer writer)
//     {
//         while (!tryPush(writer))
//             ;
//     }

//     T* front()
//     {
//         asm volatile("" : "=m"(write_idx) : :); // force read memory
//         if (read_idx == write_idx) {
//             return nullptr;
//         }
//         T* ret = &data[read_idx % CNT];
//         asm volatile("" : "=m"(data) : :); // memory fence
//         return ret;
//     }

//     void pop()
//     {
//         asm volatile("" : "=m"(data) : "m"(read_idx) :); // memory fence
//         ++read_idx;
//         asm volatile("" : : "m"(read_idx) :); // force write memory
//     }

//     template <typename Reader>
//     bool tryPop(Reader reader)
//     {
//         T* v = front();
//         if (!v)
//             return false;
//         reader(v);
//         pop();
//         return true;
//     }

// private:
//     alignas(128) T data[CNT] = {};

//     alignas(128) uint32_t write_idx = 0;
//     uint32_t read_idx_cach = 0; // used only by writing thread

//     alignas(128) uint32_t read_idx = 0;
// };

// template <class T, uint32_t CNT>
// class SPSCQueue {
// public:
//     static_assert(CNT && !(CNT & (CNT - 1)), "CNT must be a power of 2");

//     T* alloc()
//     {
//         if (free_write_cnt == 1) {
//             asm volatile("" : "=m"(read_idx) : :); // force read memory
//             free_write_cnt = (read_idx - write_idx + CNT - 1) % CNT + 1;
//             if (free_write_cnt == 1)
//                 return nullptr;
//         }
//         return &blk[write_idx].data;
//     }

//     void push()
//     {
//         asm volatile("" : : "m"(blk) :); // memory fence
//         blk[write_idx].avail = true;
//         asm volatile("" : : "m"(blk) :); // force write fence
//         write_idx = (write_idx + 1) % CNT;
//         free_write_cnt--;
//     }

//     template <typename Writer>
//     bool tryPush(Writer writer)
//     {
//         T* p = alloc();
//         if (!p)
//             return false;
//         writer(p);
//         push();
//         return true;
//     }

//     template <typename Writer>
//     void blockPush(Writer writer)
//     {
//         while (!tryPush(writer))
//             ;
//     }

//     T* front()
//     {
//         asm volatile("" : "=m"(blk) : :); // force read memory
//         auto& cur_blk = blk[read_idx];
//         if (!cur_blk.avail)
//             return nullptr;
//         return &cur_blk.data;
//     }

//     void pop()
//     {
//         blk[read_idx].avail = false;
//         asm volatile("" : "=m"(blk) : "m"(read_idx) :); // memory fence
//         read_idx = (read_idx + 1) % CNT;
//         asm volatile("" : : "m"(read_idx) :); // force write memory
//     }

//     template <typename Reader>
//     bool tryPop(Reader reader)
//     {
//         T* v = front();
//         if (!v)
//             return false;
//         reader(v);
//         pop();
//         return true;
//     }

// private:
//     struct alignas(64) Block {
//         bool avail =
//             false; // avail will be updated by both write and read thread
//         T data;
//     } blk[CNT] = {};

//     alignas(128) uint32_t write_idx = 0; // used only by writing thread
//     uint32_t free_write_cnt = CNT;

//     alignas(128) uint32_t read_idx = 0;
// };

SPSCQueue<int, 65536> g_queue;
const int g_msgNum = 100000000;

void sendVec()
{
    for (int i = 0; i < g_msgNum;) {
        int* msg = g_queue.alloc();
        if (msg == nullptr) {
            continue;
        }
        *msg = i;
        g_queue.push();
        ++i;
    }
}

void recvVec()
{
    for (int i = 0; i < g_msgNum;) {
        int* msg = g_queue.front();
        if (msg == nullptr) {
            continue;
        }
        assert(*msg == i);
        g_queue.pop();
        ++i;
    }
}

int main()
{
    std::thread t1(sendVec);
    std::thread t2(recvVec);

    t1.join();
    t2.join();
}