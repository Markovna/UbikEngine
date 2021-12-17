#include "stream_buffers.h"

#include "base/log.h"
#include "base/crc32.h"

#include <random>
#include <fstream>

void stream_buffers::_debug_info(std::ostream &out) const {
  size_t memory_size = 0;
  for (auto& buffer : buffers_) {
    out << "  [";
    out << "size: " << buffer.size << " bytes";

    if (!buffer.path.empty())
      out << ", [" << buffer.path.c_str() << "]";

    if (!buffer.memory_ptr.expired()) {
      out << ", loaded";
    }
    out << "]\n";
    memory_size += buffer.memory_ptr.expired() ? 0 : buffer.size;
  }

  out << "Memory consumption: " << memory_size << "\n";
}


std::pair<uint64_t, std::shared_ptr<uint8_t>> stream_buffers::add(size_t size) {
  assert(size);

  auto key = buffers_.emplace(buffer_desc { .size = size, .path = {}, .offset = 0 });
  auto ptr = std::shared_ptr<uint8_t>(new uint8_t[size], std::default_delete<uint8_t[]>());
  buffers_[key].memory_ptr = ptr;
  buffers_[key].loaded_ptr = ptr; // don't delete memory until buffer is not mapped
  return { to_int(key), ptr };
}

uint64_t stream_buffers::map(fs::path path, size_t offset, size_t size) {
  assert(!path.empty());
  if (!fs::exists(path)) {
    logger::core::Error("Couldn't map buffer: invalid path {}", path.c_str());
    return 0;
  }

  if (size == 0) {
    size = fs::file_size(path);
  }

  auto key = buffers_.emplace(buffer_desc { .size = size, .path = std::move(path), .offset = offset });
  return to_int(key);
}

std::pair<size_t, std::shared_ptr<const uint8_t>> stream_buffers::get(uint64_t id) const {
  auto key = to_key(id);
  const buffer& buf = buffers_[key];
  auto ptr = buf.memory_ptr.lock();
  if (!ptr) {
    ptr = std::shared_ptr<uint8_t>(new uint8_t[buf.size], std::default_delete<uint8_t[]>());
    buffers_[key].memory_ptr = ptr;

    std::ifstream file(buf.path, std::ios::in | std::ios::out | std::ios::binary);
    if (buf.offset) file.rdbuf()->pubseekoff(buf.offset, std::ios::beg, std::ios::in | std::ios::out);
    std::copy_n(std::istreambuf_iterator<char>(file), buf.size, ptr.get());
  }

  return { buf.size, ptr };
}

void stream_buffers::save(uint64_t id, const fs::path &path, size_t offset) {
  auto key = to_key(id);
  buffer& buf = buffers_[key];

  std::ofstream dst(path, std::ios::in | std::ios::out | std::ios::binary);
  if (offset) dst.rdbuf()->pubseekoff(offset, std::ios::beg, std::ios::in | std::ios::out);

  if (!buf.memory_ptr.expired()) {
    std::copy_n(buf.memory_ptr.lock().get(), buf.size, std::ostreambuf_iterator<char>(dst));
  } else {
    std::ifstream src(buf.path, std::ios::in | std::ios::binary);
    if (buf.offset) src.rdbuf()->pubseekoff(buf.offset, std::ios::beg, std::ios::in | std::ios::out);
    std::copy_n(std::istreambuf_iterator<char>(src), buf.size, std::ostreambuf_iterator<char>(dst));
  }

  buf.path = path;
  buf.loaded_ptr.reset();
  buf.offset = offset;
}

void stream_buffers::destroy(uint64_t id) {
  auto key = to_key(id);
  buffers_.erase(key);
}

bool stream_buffers::has(uint64_t id) const {
  return buffers_.find(to_key(id)) != buffers_.end();
}

bool stream_buffers::is_loaded(uint64_t id) const {
  return !buffers_.at(to_key(id)).memory_ptr.expired();
}

bool stream_buffers::is_mapped(uint64_t id) const {
  return !buffers_.at(to_key(id)).path.empty();
}

constexpr uint64_t stream_buffers::to_int(stream_buffers::key_t key) {
  return (((uint64_t) key.second) << 32U) | ((uint64_t) key.first);
}

constexpr stream_buffers::key_t stream_buffers::to_key(uint64_t value) {
  return { value & 0xFFFFFFFFUL, value >> 32U };
}

void stream_buffers::hash_dirty(uint64_t id) {
  buffers_[to_key(id)].hash = 0;
}

uint32_t stream_buffers::hash(uint64_t id) {
  buffer& buffer = buffers_[to_key(id)];
  if (!buffer.hash) {
    auto [size, data] = get(id);
    buffer.hash = utils::crc32(data.get(), size);
  }

  return buffer.hash;
}

size_t stream_buffers::size(uint64_t id) const {
  return buffers_.find(to_key(id))->size;
}
