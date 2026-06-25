# Custom Allocator

A modern C++23 re-implementation of [Ginger Bill's Memory Allocation Strategies](https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/) series. The original articles present custom allocators in C — this project rewrites them in idiomatic, standard-compliant C++ so they integrate directly with the STL via `std::pmr::memory_resource`.

## Requirements

- **C++23**
- **Clang 22**
- **CMake 3.25+**
- **Ninja** (build system)

## How to Build and Run

```bash
# 1. Configure the project (only needed once, or after changing CMakeLists.txt)
cmake --preset debug

# 2. Build
cmake --build --preset debug

# 3. Run
./build/debug/custom_alloc
```

Or as a single command:

```bash
cmake --build --preset debug && ./build/debug/custom_alloc
```

For a release (optimized) build:

```bash
cmake --preset release
cmake --build --preset release
./build/release/custom_alloc
```

## Progress

- [x] **Basic Arena (Linear) Allocator** — `include/custom_alloc/basic_arena.hpp`
  - O(1) allocation with alignment support via `std::align`
  - In-place resize for the most recent allocation
  - Bulk `free_all()` deallocation
  - RAII `SavePoint` for temporary scoped allocations
  - Full `std::pmr::memory_resource` integration (works with `std::pmr::vector`, `std::pmr::string`, etc.)
- [ ] Stack Allocator
- [ ] Pool Allocator
- [ ] Free List Allocator

## References

- [Memory Allocation Strategies - Part 2: Linear/Arena Allocators](https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/) by Ginger Bill
