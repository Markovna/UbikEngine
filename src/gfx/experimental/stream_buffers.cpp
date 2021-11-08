#include "stream_buffers.h"

#include "base/log.h"

#include <random>
#include <fstream>

uint64_t stream_buffers::generate_id() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<uint64_t> dis;

  return dis(gen);
}

void stream_buffers::__debug_info(std::ostream &out) const {
  size_t memory_size = 0;
  for (auto& buffer : buffers_) {
    out << "  [";
    out << "size: " << buffer.size << " bytes";

    if (!buffer.path.empty())
      out << ", [" << buffer.path.c_str() << "]";

    if (buffer.memory.size()) {
      out << ", loaded";
    }
    out << "]\n";
    memory_size += buffer.memory.size();
  }

  out << "Memory consumption: " << memory_size << "\n";
}

std::pair<uint64_t, uint8_t *> stream_buffers::add(size_t size) {
  assert(size > 0);
  uint64_t id = generate_id();
  auto key = buffers_.emplace(buffer_desc { .allocate = true, .size = size, .path = {}, .offset = 0 });
  id_index_[id] = key;
  return { id, buffers_[key].memory.raw() };
}

uint64_t stream_buffers::map(fs::path path, size_t offset, size_t size) {
  assert(!path.empty());
  if (!fs::exists(path)) {
    logger::core::Error("Couldn't map buffer: invalid path {}", path.c_str());
    return 0;
  }

  if (size == 0) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    size = file.rdbuf()->pubseekoff(0, std::ios::end, std::ios_base::in);
  }

  uint64_t id = generate_id();
  auto key = buffers_.emplace(buffer_desc { .allocate = false, .size = size, .path = std::move(path), .offset = offset });
  id_index_[id] = key;
  return id;
}

std::pair<size_t, const uint8_t *> stream_buffers::get(uint64_t id) const {
  auto it = id_index_.find(id);
  if (it == id_index_.end())
    return { 0, nullptr };

  const buffer& buf = buffers_[it->second];
  if (buf.memory.size() == 0) {
    assert(!buf.path.empty());

    buf.memory = memory(buf.size);

    std::ifstream file(buf.path, std::ios::in | std::ios::out | std::ios::binary);
    if (buf.offset) file.rdbuf()->pubseekoff(buf.offset, std::ios::beg, std::ios::in | std::ios::out);
    std::copy_n(std::istreambuf_iterator<char>(file), buf.size, buf.memory.begin());
  }
  return { buf.memory.size(), buf.memory.raw() };
}

void stream_buffers::save(uint64_t id, const fs::path &path, size_t offset) {
  auto it = id_index_.find(id);
  buffer& buf = buffers_[it->second];

  std::ofstream dst(path, std::ios::in | std::ios::out | std::ios::binary);
  if (offset) dst.rdbuf()->pubseekoff(offset, std::ios::beg, std::ios::in | std::ios::out);

  if (buf.memory.size() > 0) {
    std::copy_n(buf.memory.begin(), buf.size, std::ostreambuf_iterator<char>(dst));
  } else {
    std::ifstream src(buf.path, std::ios::in | std::ios::binary);
    if (buf.offset) src.rdbuf()->pubseekoff(buf.offset, std::ios::beg, std::ios::in | std::ios::out);
    std::copy_n(std::istreambuf_iterator<char>(src), buf.size, std::ostreambuf_iterator<char>(dst));
  }

  buf.path = path;
  buf.offset = offset;
}

void stream_buffers::unload(uint64_t id) const {
  auto it = id_index_.find(id);
  const buffer& buf = buffers_[it->second];
  assert(!buf.path.empty());

  if (buf.memory.size() > 0) {
    buf.memory = memory(0);
  }
}

void stream_buffers::destroy(uint64_t id) {
  auto it = id_index_.find(id);
  buffers_.erase(it->second);
  id_index_.erase(it);
}

bool stream_buffers::has(uint64_t id) const {
  return id_index_.find(id) != id_index_.end();
}

bool stream_buffers::is_loaded(uint64_t id) const {
  auto it = id_index_.find(id);
  return buffers_.at(it->second).memory.size() > 0;
}

bool stream_buffers::is_mapped(uint64_t id) const {
  auto it = id_index_.find(id);
  return !buffers_.at(it->second).path.empty();
}
