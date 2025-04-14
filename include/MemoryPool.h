#pragma once 

#include <atomic>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>

namespace Kama_memoryPool
{
#define MEMORY_POOL_NUM 64
#define SLOT_BASE_SIZE 8
#define MAX_SLOT_SIZE 512

/* The actual slot size of each memory pool cannot be determined in advance,
   because each memory pool has a different slot size (multiples of 8).
   Therefore, sizeof(this struct) is not the actual slot size. */
struct Slot 
{
    std::atomic<Slot*> next; // Atomic pointer
};

class MemoryPool
{
public:
    MemoryPool(size_t BlockSize = 4096);
    ~MemoryPool();
    
    void init(size_t);

    void* allocate();
    void deallocate(void*);
private:
    void allocateNewBlock();
    size_t padPointer(char* p, size_t align);

    // Use CAS (Compare And Swap) operations for lock-free enqueue and dequeue
    bool pushFreeList(Slot* slot);
    Slot* popFreeList();
private:
    int                 BlockSize_; // Size of each memory block
    int                 SlotSize_; // Size of each slot
    Slot*               firstBlock_; // Points to the first memory block managed by the pool
    Slot*               curSlot_; // Points to the current unused slot
    std::atomic<Slot*>  freeList_; // Points to the free slot (slots that were used and then released)
    Slot*               lastSlot_; // Indicates the last available position in the current memory block (beyond this, a new block is needed)
    //std::mutex          mutexForFreeList_; // Ensures atomic operations on freeList_ in multi-threaded scenarios
    std::mutex          mutexForBlock_; // Prevents unnecessary repeated memory allocations in multi-threaded scenarios
};

class HashBucket
{
public:
    static void initMemoryPool();
    static MemoryPool& getMemoryPool(int index);

    static void* useMemory(size_t size)
    {
        if (size <= 0)
            return nullptr;
        if (size > MAX_SLOT_SIZE) // For memory larger than 512 bytes, use new
            return operator new(size);

        // Equivalent to ceil(size / 8) (since allocated memory must not be smaller than requested)
        return getMemoryPool(((size + 7) / SLOT_BASE_SIZE) - 1).allocate();
    }

    static void freeMemory(void* ptr, size_t size)
    {
        if (!ptr)
            return;
        if (size > MAX_SLOT_SIZE)
        {
            operator delete(ptr);
            return;
        }

        getMemoryPool(((size + 7) / SLOT_BASE_SIZE) - 1).deallocate(ptr);
    }

    template<typename T, typename... Args> 
    friend T* newElement(Args&&... args);
    
    template<typename T>
    friend void deleteElement(T* p);
};

template<typename T, typename... Args>
T* newElement(Args&&... args)
{
    T* p = nullptr;
    // Select an appropriate memory pool based on the element size
    if ((p = reinterpret_cast<T*>(HashBucket::useMemory(sizeof(T)))) != nullptr)
        // Construct the object in the allocated memory
        new(p) T(std::forward<Args>(args)...);

    return p;
}

template<typename T>
void deleteElement(T* p)
{
    // Call the destructor
    if (p)
    {
        p->~T();
        // Recycle the memory
        HashBucket::freeMemory(reinterpret_cast<void*>(p), sizeof(T));
    }
}

} // namespace memoryPool
