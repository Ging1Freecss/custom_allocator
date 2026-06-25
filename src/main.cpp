#include "custom_alloc/basic_arena.hpp"
#include <cstddef>
#include <iostream>
#include <vector>

void Testing_Basic_Arena() {
  // with pointers
  {
    std::cout << "with pointers" << std::endl;
    alignas(64) std::byte buffer[1024];

    custom_alloc::BasicArena arena{buffer};

    int *x{static_cast<int *>(arena.allocate_align(sizeof(int), alignof(int)))};
    *x = 10;
    std::cout << *x << std::endl;
  }

  // with vector
  {
    std::cout << "with vector" << std::endl;
    alignas(64) std::byte buffer[4096];
    custom_alloc::BasicArena arena(buffer);
    // Pass the arena's address to a PMR vector
    std::pmr::vector<int> numbers(&arena);
    numbers.push_back(1);
    numbers.push_back(2);
    numbers.push_back(3);

    for (auto e : numbers) {
      std::cout << e << " ";
    }
    std::cout << std::endl;
  }

  // using SavePoint
  {
    std::cout << "using SavePoint" << std::endl;

    alignas(64) std::byte buffer[2048];
    custom_alloc::BasicArena arena(buffer);
    // Permanent allocation
    int *keep =
        static_cast<int *>(arena.allocate_align(sizeof(int), alignof(int)));
    *keep = 100;
    {
      // Create a savepoint — captures the current offset state
      custom_alloc::BasicArena::SavePoint guard(arena);
      // Temporary allocations
      char *tmp = static_cast<char *>(arena.allocate_align(256, 1));
      *tmp = 'a';
      std::cout << *tmp << std::endl;
    }
    std::cout << *keep << std::endl;
  }

  // resize logic
  {
    std::cout << "using Resize" << std::endl;

    alignas(64) std::byte buffer[2048];
    custom_alloc::BasicArena arena(buffer);

    int *ptr =
        static_cast<int *>(arena.allocate_align(sizeof(int), alignof(int)));
    *ptr = 10;

    std::cout << "old memory" << std::endl;
    std::cout << *ptr << std::endl;
    ptr = static_cast<int *>(arena.resize_align(
        static_cast<void *>(ptr), sizeof(int), 2 * sizeof(int), alignof(int)));

    *ptr = 11;
    int *next_ptr = ptr + 1;
    *next_ptr = 12;

    std::cout << "new memory" << std::endl;
    std::cout << *ptr << std::endl;
    std::cout << *next_ptr << std::endl;
  }
}
int main() { Testing_Basic_Arena(); }
