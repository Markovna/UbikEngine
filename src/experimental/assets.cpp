#include "assets.h"

#include <fstream>
#include <set>

namespace experimental::assets {

bool read(const fs::path &path, asset &data) {
  std::ifstream file = fs::read_file(path, std::ios::in);
  if (!file)
    return false;

  data = asset::parse(file);
  return true;
}

void write(const fs::path &path, const asset &asset) {
  std::ofstream file(path, std::ios::trunc);
  file << asset.dump(2);
}

handle::handle(repository* repository, const repository::key& key)
  : repository_(repository), key_(key) {
  ++get().use_count;
}

handle::~handle() {
  assert(get().use_count);
  uint32_t count = --get().use_count;
  if (!count) {
    repository_->erase(key_);
  }
}

handle::handle(const handle& other) : repository_(other.repository_), key_(other.key_) {
  ++get().use_count;
}

handle::handle(handle&& other) noexcept : repository_(other.repository_), key_(other.key_) {
  other.repository_ = nullptr;
}

handle& handle::operator=(handle&& other) noexcept {
  handle(std::move(other)).swap(*this);
  return *this;
}

handle& handle::operator=(const handle& other) {
  handle(other).swap(*this);
  return *this;
}

void handle::swap(handle& other) noexcept {
  std::swap(repository_, other.repository_);
  std::swap(key_, other.key_);
}

asset filesystem_provider::load(const fs::path& path) {
  asset asset;
  read(path, asset);
  return asset;
}

fs::path filesystem_provider::get_path(const guid& id) {
  auto iterator = guid_to_path_.find(id);
  if (iterator == guid_to_path_.end())
    return {};

  return iterator->second;
}

void filesystem_provider::save(const fs::path& path, const asset& asset) {
  guid id = guid::from_string(asset["__guid"]);
  guid_to_path_[id] = path;
  write(path, asset);
}

void filesystem_provider::reload(repository& repository) {
  // TODO: check for all changes and load it to repository
//  repository.replace()
}

void filesystem_provider::load_buffer(const fs::path& path, uint64_t buffer_id, std::ostream& stream) {
  fs::path buffer_path = fs::append(fs::concat(path, ".buffers"), std::to_string(buffer_id));
  std::ifstream file = fs::read_file(buffer_path);

  size_t size = 0;
  file.read((char*) &size, sizeof(size));

  char buf[size];
  file.read(buf, size);

  stream.write((char*) buf, size);
}

void filesystem_provider::add(const fs::path& path) {
  static const std::set<std::string> extensions = {
      ".meta",
      ".entity"
  };

  for (auto it = fs::recursive_directory_iterator(path); it != fs::recursive_directory_iterator(); it++) {
    if (it->is_directory())
      continue;

    if (!extensions.count(it->path().extension()))
      continue;

    asset asset;
    if (!read(it->path(), asset))
      continue;

    guid id = guid::from_string(asset["__guid"]);
    if (id.is_valid()) {
      guid_to_path_[id] = it->path();
    }
  }
}

}