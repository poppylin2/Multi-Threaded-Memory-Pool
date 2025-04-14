
# Memory Pool

A high-performance, multi-threaded memory pool for efficient small object allocation and deallocation.

## Purpose

This memory pool is designed to solve the performance issues caused by frequent dynamic memory allocation and deallocation in multi-threaded applications, especially when handling a large number of small objects.

Specifically, it:
- Reduces the overhead of frequent `new` and `delete` calls.
- Minimizes memory fragmentation by reusing fixed-size memory slots.
- Supports multi-threaded environments with lock-free free list management.
- Provides better performance for small object allocation (≤ 512 bytes), while larger allocations fall back to standard heap allocation.

## Features

- Lock-free free list for fast memory management.
- Slot size granularity of 8 bytes, up to a maximum of 512 bytes.
- 64 individual memory pools for different object sizes.
- Template-based allocation and deallocation interface.
- Benchmarking code provided for performance comparison.

## Project Structure

- `MemoryPool`  
  Manages memory blocks and slots, performs allocation and deallocation.

- `HashBucket`  
  Provides 64 memory pools, selecting the appropriate pool based on object size.

- Template Helpers  
  - `newElement<T>()`: Allocate and construct an object.
  - `deleteElement<T>()`: Destruct and deallocate an object.

- Benchmarking  
  Functions to compare performance between memory pool and standard allocation.

## Usage

### 1. Initialization

Before using the memory pool, initialize it at the beginning of your program:

```cpp
HashBucket::initMemoryPool();
```

### 2. Allocation and Deallocation

Replace `new` and `delete` with:

```cpp
MyClass* obj = newElement<MyClass>();
deleteElement(obj);
```

### 3. Benchmark

Run the benchmark functions to compare performance:

```cpp
BenchmarkMemoryPool(100, 1, 10); // Memory pool benchmark
BenchmarkNew(100, 1, 10);        // Standard new/delete benchmark
```

Sample output:

```
1 threads running 10 rounds concurrently, 100 allocations & deallocations per round, total time: XXX ms
===============================================================================
===============================================================================
1 threads running 10 rounds concurrently, 100 malloc & free operations per round, total time: XXX ms
```

## Build and Run

### Recommended: Build with VSCode

1. Install required VSCode extensions:
   - CMake Tools
   - C/C++ (by Microsoft)

2. Install a C++ compiler:
   - MinGW-w64 (recommended for Windows)
   - Make sure `g++` and `cmake` are added to your system PATH.

3. Open the project folder in VSCode.

4. In the bottom status bar, click `[No Kit Selected]` and select your compiler (e.g., GCC).

5. Press `Ctrl + Shift + P`, run:
   - `CMake: Configure`
   - `CMake: Build`

6. Run the executable:
   - Either press `Ctrl + Shift + P` and run `CMake: Run Without Debugging`
   - Or open terminal and run:
     ```bash
     ./build/Debug/MemoryPoolProject.exe
     ```

### Alternative: Command line build (if environment is configured)

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
./MemoryPoolProject.exe
```

> Note: If you use Visual Studio, you can generate the Visual Studio project with:
> ```bash
> cmake .. -G "Visual Studio 17 2022"
> ```

## When to Use

- High-frequency creation and destruction of small objects.
- Performance-sensitive applications such as game engines, servers, and real-time systems.
- Multi-threaded programs that require efficient memory management.

## Notes

- Objects larger than 512 bytes are allocated from the standard heap.
- Before using the memory pool, `HashBucket::initMemoryPool()` must be called.
- Multi-threaded safe allocation and deallocation are supported.
