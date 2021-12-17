#pragma once

#include <vector>
#include <unordered_map>

#include "platform/file_system.h"
#include "base/slot_map.h"

class stream_buffers {
 private:
  struct buffer_desc {
    size_t size;
    fs::path path;
    size_t offset;
  };

  struct buffer {
    explicit buffer(buffer_desc desc)
        : memory_ptr()
        , path(std::move(desc.path))
        , offset(desc.offset)
        , size(desc.size)
        , hash(0)
    {}

    mutable std::weak_ptr<uint8_t> memory_ptr;
    mutable std::shared_ptr<uint8_t> loaded_ptr;
    fs::path path;
    size_t offset;
    size_t size;
    uint32_t hash;
  };

  using key_t = stdext::slot_map<struct any>::key_type;

 public:
  void _debug_info(std::ostream& out) const;

  stream_buffers() = default;
  stream_buffers(const stream_buffers&) = default;
  stream_buffers& operator=(const stream_buffers&) = default;

  std::pair<uint64_t, std::shared_ptr<uint8_t>> add(size_t size);
  uint64_t map(fs::path path, size_t offset = 0, size_t size = 0);
  [[nodiscard]] std::pair<size_t, std::shared_ptr<const uint8_t>> get(uint64_t id) const;

  void hash_dirty(uint64_t id);
  uint32_t hash(uint64_t id);
  void save(uint64_t id, const fs::path& path, size_t offset);
  void destroy(uint64_t id);

  [[nodiscard]] bool has(uint64_t id) const;
  [[nodiscard]] size_t size(uint64_t id) const;
  [[nodiscard]] bool is_loaded(uint64_t id) const;
  [[nodiscard]] bool is_mapped(uint64_t id) const;

 private:
  constexpr static inline uint64_t to_int(key_t);
  constexpr static inline key_t to_key(uint64_t);

 private:
  stdext::slot_map<buffer> buffers_;
};

