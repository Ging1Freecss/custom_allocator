
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <memory_resource>
#include <span>

namespace custom_alloc {
class BasicArena : public std::pmr::memory_resource {
private:
  std::span<std::byte> m_buffer;
  std::size_t m_curr_offset;
  std::size_t m_prev_offset;

public:
  explicit BasicArena(std::span<std::byte> buffer)
      : m_buffer{buffer}, m_curr_offset{0}, m_prev_offset{0} {}

  BasicArena(const BasicArena &arena) = delete;
  auto operator=(const BasicArena &arena) -> BasicArena & = delete;

  BasicArena(BasicArena &&arena) {
    if (&arena == this) {
      return;
    }

    m_buffer = std::move(arena.m_buffer);
    m_curr_offset = arena.m_curr_offset;
    m_prev_offset = arena.m_prev_offset;

    arena.m_curr_offset = 0;
    arena.m_prev_offset = 0;
  }
  auto operator=(BasicArena &&arena) -> BasicArena & {
    if (&arena == this) {
      return *this;
    }

    m_buffer = std::move(arena.m_buffer);
    m_curr_offset = arena.m_curr_offset;
    m_prev_offset = arena.m_prev_offset;

    arena.m_curr_offset = 0;
    arena.m_prev_offset = 0;
    arena.m_buffer = {};
    return *this;
  }

  // allocate memory with specifix alignment
  auto allocate_align(std::size_t size, std::size_t alignment) -> void * {

    if (m_curr_offset >= m_buffer.size()) {
      return nullptr;
    }
    std::size_t remaining_space{m_buffer.size() - m_curr_offset};
    void *current_ptr{m_buffer.data() + m_curr_offset};

    if (std::align(alignment, size, current_ptr, remaining_space)) {
      if (static_cast<std::byte *>(current_ptr) - m_buffer.data() >= 0) {
        std::size_t align_offset{static_cast<std::size_t>(
            static_cast<std::byte *>(current_ptr) - m_buffer.data())};

        m_prev_offset = align_offset;
        m_curr_offset = align_offset + size;

        std::memset(current_ptr, 0, size);

        return current_ptr;
      }
    }

    return nullptr;
  }

  // Resize an existing allocation
  auto resize_align(void *old_memory, std::size_t old_size,
                    std::size_t new_size, std::size_t alignment) -> void * {

    if (!old_memory || old_size == 0) {
      return allocate_align(new_size, alignment);
    }

    if (m_prev_offset + new_size > m_buffer.size())
      return nullptr;

    std::byte *old_byte_ptr{static_cast<std::byte *>(old_memory)};

    //  optimisation if we are resizing the memory block
    // m_prev_offset and m_curr_offset
    if (old_byte_ptr == m_buffer.data() + m_prev_offset) {
      m_curr_offset = m_prev_offset + new_size;

      if (new_size > old_size) {
        std::memset(old_byte_ptr + old_size, 0, new_size - old_size);
      }

      return old_memory;
    }

    // resizing of older allocation
    std::size_t remaining_space{m_buffer.size() - m_curr_offset};
    void *current_ptr{m_buffer.data() + m_curr_offset};

    if (!std::align(alignment, new_size, current_ptr, remaining_space)) {
      return nullptr;
    }

    std::memmove(current_ptr, old_memory, old_size);

    // modify offset
    std::size_t align_offset{static_cast<std::size_t>(
        static_cast<std::byte *>(current_ptr) - m_buffer.data())};

    m_prev_offset = align_offset;
    m_curr_offset = align_offset + new_size;

    // imp to zero initialised
    if (new_size > old_size) {
      std::memset(static_cast<std::byte *>(current_ptr) + old_size, 0,
                  new_size - old_size);
    }
    return current_ptr;
  }

  // Getters for capacity and usage details
  [[nodiscard]] std::size_t capacity() const noexcept {
    return m_buffer.size();
  }
  [[nodiscard]] std::size_t used_memory() const noexcept {
    return m_curr_offset;
  }

  void free_all() noexcept {
    m_curr_offset = 0;
    m_prev_offset = 0;
  }

  class [[nodiscard]] SavePoint {
  private:
    BasicArena &m_arena;
    std::size_t m_saved_curr;
    std::size_t m_saved_prev;

  public:
    explicit SavePoint(BasicArena &arena)
        : m_arena(arena), m_saved_curr(arena.m_curr_offset),
          m_saved_prev(arena.m_prev_offset) {}

    SavePoint(const SavePoint &) = delete;
    SavePoint(const SavePoint &&) = delete;
    auto operator=(const SavePoint &) -> SavePoint & = delete;
    auto operator=(SavePoint &&) -> SavePoint & = delete;

    ~SavePoint() {
      m_arena.m_curr_offset = m_saved_curr;
      m_arena.m_prev_offset = m_saved_prev;
    }
  };

  friend SavePoint;
  virtual ~BasicArena() override { free_all(); }

protected:
  virtual void *do_allocate(std::size_t size, std::size_t alignment) override {
    if (void *ptr = allocate_align(size, alignment); ptr != nullptr) {
      return ptr;
    }

    throw std::bad_alloc();
  }

  virtual void do_deallocate(void *, std::size_t, std::size_t) override {
    // Linear allocator does not support individual deallocation (no-op)
  }

  virtual bool
  do_is_equal(const std::pmr::memory_resource &other) const noexcept override {
    return this == &other;
  }
};
} // namespace custom_alloc