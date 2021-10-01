#include "assets.h"

#include <fstream>
#include <set>
#include <base/log.h>

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

handle details::load(provider* provider, repository &rep, const guid &id) {
  if (auto [ key, success ] = rep.find(id); success)
    return { &rep, key };

  fs::path path = provider->get_path(id);
  repository::key key = rep.emplace(id, path);
  rep[key].asset = provider->load(path);
  return { &rep, key };
}

handle details::load(provider* provider, repository &rep, const fs::path &path) {
  if (auto [ key, success ] = rep.find(path); success)
    return { &rep, key };

  asset asset = provider->load(path);
  repository::key key = rep.emplace(guid::from_string(asset["__guid"]), path);
  rep[key].asset = std::move(asset);
  return { &rep, key };
}

handle load(provider* provider, const guid& id) {
  return details::load(provider, *g_repository, id);
}

handle load(provider* provider,const fs::path& path) {
  return details::load(provider, *g_repository, path);
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

void init() {
  g_repository = new repository;
}

void shutdown() {
  delete g_repository;
}

handle::handle(repository* repository, const repository::key& key)
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

}