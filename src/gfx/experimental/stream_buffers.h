#pragma once

#include <vector>
#include <unordered_map>

#include "platform/file_system.h"
#include "base/slot_map.h"

class stream_buffers {
 private:
  class memory {
   public:
    explicit memory(size_t size, uint8_t fill = 0) : data(size, fill) {}

    inline const uint8_t* raw() const { return data.data(); }
    inline uint8_t* raw() { return data.data(); }

    inline size_t size() const { return data.size(); }

    inline auto begin() { return data.begin(); }
    inline auto end() { return data.end(); }

   private:
    std::vector<uint8_t> data;
  };

  struct buffer_desc {
    bool allocate;
    size_t size;
    fs::path path;
    size_t offset;
  };

  struct buffer {
    explicit buffer(buffer_desc desc)
        : memory(desc.allocate ? desc.size : 0)
        , path(std::move(desc.path))
        , offset(desc.offset)
        , size(desc.size)
    {}

    mutable memory memory;
    fs::path path;
    size_t offset;
    size_t size;
  };

  using key_t = stdext::slot_map<struct any>::key_type;

  static uint64_t generate_id();

 public:
  void __debug_info(std::ostream& out) const;

  stream_buffers() = default;
  stream_buffers(const stream_buffers&) = default;
  stream_buffers& operator=(const stream_buffers&) = default;

  std::pair<uint64_t, uint8_t*> add(size_t size);
  uint64_t map(fs::path path, size_t offset = 0, size_t size = 0);
  std::pair<size_t, const uint8_t*> get(uint64_t id) const;

  void save(uint64_t id, const fs::path& path, size_t offset);
  void unload(uint64_t id) const;
  void destroy(uint64_t id);

  [[nodiscard]] bool has(uint64_t id) const;
  [[nodiscard]] bool is_loaded(uint64_t id) const;
  [[nodiscard]] bool is_mapped(uint64_t id) const;

 private:
  stdext::slot_map<buffer> buffers_;
  std::unordered_map<uint64_t, key_t> id_index_;
};

