#include "../include/MemoryPool.h"

namespace Kama_memoryPool 
{
MemoryPool::MemoryPool(size_t BlockSize)
    : BlockSize_ (BlockSize)
    , SlotSize_ (0)
    , firstBlock_ (nullptr)
    , curSlot_ (nullptr)
    , freeList_ (nullptr)
    , lastSlot_ (nullptr)
{}

MemoryPool::~MemoryPool()
{
    // Delete all the allocated memory blocks
    Slot* cur = firstBlock_;
    while (cur)
    {
        Slot* next = cur->next;
        // Equivalent to free(reinterpret_cast<void*>(firstBlock_));
        // Convert to void pointer since void type does not require destructor, only memory release
        operator delete(reinterpret_cast<void*>(cur));
        cur = next;
    }
}

void MemoryPool::init(size_t size)
{
    assert(size > 0);
    SlotSize_ = size;
    firstBlock_ = nullptr;
    curSlot_ = nullptr;
    freeList_ = nullptr;
    lastSlot_ = nullptr;
}

void* MemoryPool::allocate()
{
    // First try to use memory slots from the free list
    Slot* slot = popFreeList();
    if (slot != nullptr)
        return slot;

    Slot* temp;
    {   
        std::lock_guard<std::mutex> lock(mutexForBlock_);
        if (curSlot_ >= lastSlot_)
        {
            // If the current block has no available slots, allocate a new block
            allocateNewBlock();
        }
    
        temp = curSlot_;
        // Can't do curSlot_ += SlotSize_ directly because curSlot_ is Slot* type,
        // need to divide by Slot size and then increment
        curSlot_ += SlotSize_ / sizeof(Slot);
    }
    
    return temp; 
}

void MemoryPool::deallocate(void* ptr)
{
    if (!ptr) return;

    Slot* slot = reinterpret_cast<Slot*>(ptr);
    pushFreeList(slot);
}

void MemoryPool::allocateNewBlock()
{   
    //std::cout << "Allocating a new block, SlotSize: " << SlotSize_ << std::endl;
    // Use head insertion to add the new memory block
    void* newBlock = operator new(BlockSize_);
    reinterpret_cast<Slot*>(newBlock)->next = firstBlock_;
    firstBlock_ = reinterpret_cast<Slot*>(newBlock);

    char* body = reinterpret_cast<char*>(newBlock) + sizeof(Slot*);
    size_t paddingSize = padPointer(body, SlotSize_); // Calculate padding for alignment
    curSlot_ = reinterpret_cast<Slot*>(body + paddingSize);

    // If exceeded this marker, it means the block has no available slots, need to allocate a new block
    lastSlot_ = reinterpret_cast<Slot*>(reinterpret_cast<size_t>(newBlock) + BlockSize_ - SlotSize_ + 1);

    freeList_ = nullptr;
}

// Align the pointer to the multiple of slot size
size_t MemoryPool::padPointer(char* p, size_t align)
{
    // align is the slot size
    return (align - reinterpret_cast<size_t>(p)) % align;
}

// Implement lock-free enqueue operation
bool MemoryPool::pushFreeList(Slot* slot)
{
    while (true)
    {
        // Get the current head node
        Slot* oldHead = freeList_.load(std::memory_order_relaxed);
        // Set the next of the new node to point to the current head node
        slot->next.store(oldHead, std::memory_order_relaxed);

        // Try to set the new node as the new head
        if (freeList_.compare_exchange_weak(oldHead, slot,
         std::memory_order_release, std::memory_order_relaxed))
        {
            return true;
        }
        // Failure: another thread might have modified freeList_
        // Retry if CAS fails
    }
}

// Implement lock-free dequeue operation
Slot* MemoryPool::popFreeList()
{
    while (true)
    {
        Slot* oldHead = freeList_.load(std::memory_order_acquire);
        if (oldHead == nullptr)
            return nullptr; // Queue is empty

        // Validate oldHead before accessing newHead
        Slot* newHead = nullptr;
        try
        {
            newHead = oldHead->next.load(std::memory_order_relaxed);
        }
        catch(...)
        {
            // If fails, retry memory allocation
            continue;
        }
        
        // Attempt to update the head node
        // Atomically update freeList_ from oldHead to newHead
        if (freeList_.compare_exchange_weak(oldHead, newHead,
         std::memory_order_acquire, std::memory_order_relaxed))
        {
            return oldHead;
        }
        // Failure: another thread might have modified freeList_
        // Retry if CAS fails
    }
}

void HashBucket::initMemoryPool()
{
    for (int i = 0; i < MEMORY_POOL_NUM; i++)
    {
        getMemoryPool(i).init((i + 1) * SLOT_BASE_SIZE);
    }
}   

// Singleton pattern
MemoryPool& HashBucket::getMemoryPool(int index)
{
    static MemoryPool memoryPool[MEMORY_POOL_NUM];
    return memoryPool[index];
}

} // namespace memoryPool
