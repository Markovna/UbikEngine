#pragma once

#include <string>

class allocator {
 public:
  virtual void* realloc(void* ptr, size_t size, size_t align) = 0;
  virtual ~allocator() = default;
};

class allocator_default : public allocator {
 public:
  void* realloc(void* ptr, size_t size, size_t align) override;
};

inline void* alloc(allocator& allocator, size_t size, size_t align = 0) {
  return allocator.realloc(nullptr, size, align);
}

inline void* realloc(allocator& allocator, void* ptr, size_t size, size_t align = 0) {
  return allocator.realloc(ptr, size, align);
}

inline void free(allocator& allocator, void* ptr, size_t align = 0) {
  allocator.realloc(ptr, 0, align);
}


