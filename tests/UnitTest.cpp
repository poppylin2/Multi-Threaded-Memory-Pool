#include <iostream>
#include <thread>
#include <vector>

#include "../include/MemoryPool.h"

using namespace Kama_memoryPool;

// Test classes
class P1 
{
    int id_;
};

class P2 
{
    int id_[5];
};

class P3
{
    int id_[10];
};

class P4
{
    int id_[20];
};

// Single round allocation and deallocation count, number of threads, number of rounds
void BenchmarkMemoryPool(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks); // Thread pool
	size_t total_costtime = 0;
	for (size_t k = 0; k < nworks; ++k) // Create nworks threads
	{
		vthread[k] = std::thread([&]() {
			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
                    P1* p1 = newElement<P1>(); // Memory pool interface
                    deleteElement<P1>(p1);
                    P2* p2 = newElement<P2>();
                    deleteElement<P2>(p2);
                    P3* p3 = newElement<P3>();
                    deleteElement<P3>(p3);
                    P4* p4 = newElement<P4>();
                    deleteElement<P4>(p4);
				}
				size_t end1 = clock();

				total_costtime += end1 - begin1;
			}
		});
	}
	for (auto& t : vthread)
	{
		t.join();
	}
	printf("%lu threads running %lu rounds concurrently, %lu allocations & deallocations per round, total time: %lu ms\n", nworks, rounds, ntimes, total_costtime);
}

void BenchmarkNew(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	size_t total_costtime = 0;
	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
                    P1* p1 = new P1;
                    delete p1;
                    P2* p2 = new P2;
                    delete p2;
                    P3* p3 = new P3;
                    delete p3;
                    P4* p4 = new P4;
                    delete p4;
				}
				size_t end1 = clock();
				
				total_costtime += end1 - begin1;
			}
		});
	}
	for (auto& t : vthread)
	{
		t.join();
	}
	printf("%lu threads running %lu rounds concurrently, %lu malloc & free operations per round, total time: %lu ms\n", nworks, rounds, ntimes, total_costtime);
}

int main()
{
    HashBucket::initMemoryPool(); // Must call this function before using the memory pool
	BenchmarkMemoryPool(100, 1, 10); // Test memory pool
	std::cout << "===========================================================================" << std::endl;
	std::cout << "===========================================================================" << std::endl;
	BenchmarkNew(100, 1, 10); // Test new & delete
	
	return 0;
}
