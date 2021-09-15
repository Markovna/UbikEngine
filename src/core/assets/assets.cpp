#include "assets.h"

#include <fstream>
#include <set>
#include <base/log.h>

namespace assets {

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

handle load(provider* provider, repository &rep, const guid &id) {
  if (auto [ key, success ] = rep.find(id); success)
    return { &rep, key };

  fs::path path = provider->get_path(id);
  repository::key key = rep.emplace(id, path);
  rep[key].asset = provider->load(path);
  return { &rep, key };
}

handle load(provider* provider, repository &rep, const fs::path &path) {
  if (auto [ key, success ] = rep.find(path); success)
    return { &rep, key };

  asset asset = provider->load(path);
  repository::key key = rep.emplace(guid::from_string(asset["__guid"]), path);
  rep[key].asset = std::move(asset);
  return { &rep, key };
}

handle load(provider* provider, const guid& id) {
  return load(provider, *g_repository, id);
}

handle load(provider* provider,const fs::path& path) {
  return load(provider, *g_repository, path);
}

buffer_t load_buffer(provider* provider, const handle& handle, const char* name) {
  if (!handle)
    return { };

  std::stringstream stream;
  uint64_t id = handle->at(name);
  const fs::path& p = handle.path();
  provider->load_buffer(p, id, stream);
  return std::make_unique<std::stringstream>(std::move(stream));
}

uint64_t add_buffer(handle& handle, const char* name) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<uint64_t> dis;

  uint64_t buffer_id = dis(gen);
  (*handle)[name] = buffer_id;
  return buffer_id;
}

repository* g_repository;
provider* g_provider;

void init() {
  g_repository = new repository;
}

void shutdown() {
  delete g_repository;
}

void init_provider(provider* provider) {
  g_provider = provider;
}

void shutdown_provider() {
  delete g_provider;
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
  fs::path buffer_path = fs::append(get_or_create_buffers_dir(path), std::to_string(buffer_id));
  std::ifstream file = fs::read_file(buffer_path);
  stream << file.rdbuf();
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

void filesystem_provider::save_buffer(const fs::path& path, uint64_t buffer_id, const std::istream& buffer) {
  logger::core::Info("filesystem_provider::save_buffer {0}:{1}", path.c_str(), buffer_id);
  fs::path buffer_path = fs::append(get_or_create_buffers_dir(path), std::to_string(buffer_id));
  std::ofstream file(buffer_path, std::ios::binary | std::ios::trunc);
  file << buffer.rdbuf();
}

void filesystem_provider::remove_buffer(const fs::path& path, uint64_t buffer_id) {
  fs::path buffer_path = fs::append(get_or_create_buffers_dir(path), std::to_string(buffer_id));
  fs::remove(buffer_path);
}

fs::path filesystem_provider::get_or_create_buffers_dir(const fs::path& path) {
  fs::path buffers_path = fs::concat(path, ".buffers");
  if (!fs::exists(buffers_path)) {
    fs::create_directory(buffers_path);
  }
  return buffers_path;
}

}