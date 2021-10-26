#include "allocator.h"

void* allocator_default::realloc(void* ptr, size_t size, size_t align) {
  if (!size) {
    if (ptr) {
      ::free(ptr);
    }
    return nullptr;
  } else if (!ptr) {
    return align <= alignof(std::max_align_t) ? ::malloc(size) : ::aligned_alloc(size, align);
  }

  if (align <= alignof(std::max_align_t)) {
    return ::realloc(ptr, size);
  }

  ::free(ptr);
  return ::aligned_alloc(size, align);
}