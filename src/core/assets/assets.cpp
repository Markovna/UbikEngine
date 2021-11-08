#include "assets.h"

#include <fstream>

namespace assets {

bool read(const fs::path &path, asset &data) {
  std::ifstream file = fs::read_file(path, std::ios::in);
  if (!file)
    return false;

  try {
    data = asset::parse(file);
  }
  catch (asset::parse_error& ex) {
    return false;
  }

  return true;
}

void write(const fs::path &path, const asset &asset) {
  std::ofstream file(path, std::ios::trunc);
  file << asset.dump(2);
}

handle repository::load(const guid &id) {
  if (auto [ key, success ] = find(id); success)
    return { this, key };

  fs::path path = provider_->get_path(id);
  key key = emplace(id, path);
  assets_[key].asset = provider_->load(path);
  return { this, key };
}

handle repository::load(const fs::path &path) {
  if (auto [ key, success ] = find(path); success)
    return { this, key };

  asset asset = provider_->load(path);
  key key = emplace(guid::from_string(asset["__guid"]), path);
  assets_[key].asset = std::move(asset);
  return { this, key };
}

void repository::load_buffer(const fs::path& path, uint64_t buffer_id, std::ostream& out) const {
  provider_->load_buffer(path, buffer_id, out);
}

uint64_t add_buffer(asset& handle, std::string_view name) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<uint64_t> dis;

  uint64_t buffer_id = dis(gen);
  handle[std::string(name)] = buffer_id;
  return buffer_id;
}

handle::handle(repository* repository, const key& key)
  : repository_(repository), key_(key) {
  ++get().use_count;
}

handle::~handle() {
  if (!repository_)
    return;

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

handle::operator bool() const noexcept {
  return repository_ != nullptr && repository_->find(key_) != repository_->end();
}

reference handle::get() { return repository_->at(key_); }
const_reference handle::get() const { return repository_->at(key_); }

buffer_t handle::load_buffer(const asset& asset, std::string_view name) const {
  if (!(*this))
    return { };

  std::stringstream stream;
  uint64_t id = asset.at(name.data());
  const fs::path& p = path();
  repository_->load_buffer(p, id, stream);
  return std::make_unique<std::stringstream>(std::move(stream));
}

}