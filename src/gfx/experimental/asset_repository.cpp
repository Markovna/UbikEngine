#include "asset_repository.h"
#include "base/log.h"

#include <fstream>
#include <sstream>
#include <unordered_set>
#include <random>

namespace experimental {

bool read(const fs::path &path, asset &data) {
  std::ifstream file = fs::read_file(path, std::ios::in);
  if (!file)
    return false;

  try {
    data = asset::parse(file);
  }
  catch (asset::parse_error& ex) {
    logger::core::Error("Parsing {} failed with error: {}", path.c_str(), ex.what());
    return false;
  }

  return true;
}

void write(const fs::path &path, const asset &asset) {
  std::ofstream file(path, std::ios::trunc);
  file << asset.dump(2);

  logger::core::Info("Asset at path {} was written.", path.c_str());
}

uint64_t create_buffer_id() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<uint64_t> dis;

  return dis(gen);
}

asset* asset_handle::operator->() {
  return &repository_->get(key_);
}

asset& asset_handle::operator*() {
  return repository_->get(key_);
}

const asset* asset_handle::operator->() const {
  return &repository_->get(key_);
}

const asset& asset_handle::operator*() const {
  return repository_->get(key_);
}

const fs::path& asset_handle::path() {
  return repository_->path(key_);
}

const guid& asset_handle::id() {
  return repository_->id(key_);
}

asset_handle::operator bool() const noexcept {
  return repository_ && repository_->find(key_);
}

asset_handle::asset_handle(assets_repository* repository, const key_t& key) : repository_(repository), key_(key) {
  if (repository_) {
    repository_->inc_use_count(key_);
  }
}

asset_handle::~asset_handle() {
  if (repository_) {
    repository_->dec_use_count(key_);
  }
}

asset_handle::asset_handle(const asset_handle& other) : repository_(other.repository_), key_(other.key_) {
  if (repository_)
    repository_->inc_use_count(key_);
}

asset_handle::asset_handle(asset_handle&& other) noexcept : repository_(other.repository_), key_(other.key_) {
  other.repository_ = nullptr;
}

asset_handle& asset_handle::operator=(asset_handle&& other) noexcept {
  asset_handle(std::move(other)).swap(*this);
  return *this;
}

asset_handle& asset_handle::operator=(const asset_handle& other) {
  asset_handle(other).swap(*this);
  return *this;
}

void asset_handle::swap(asset_handle& other) noexcept {
  std::swap(repository_, other.repository_);
  std::swap(key_, other.key_);
}

asset& assets_repository::get(key_t key) {
  return storage_[key].asset;
}

const asset& assets_repository::get(key_t key) const {
  return storage_[key].asset;
}

const fs::path& assets_repository::path(key_t key) {
  return storage_[key].path;
}

const guid& assets_repository::id(key_t key) {
  return storage_[key].guid;
}

asset* assets_repository::find(key_t key) {
 auto it = storage_.find(key);
 return it != storage_.end() ? &it->asset : nullptr;
}

const asset* assets_repository::find(key_t key) const {
  auto it = storage_.find(key);
  return it != storage_.end() ? &it->asset : nullptr;
}

void assets_repository::inc_use_count(key_t) {}
void assets_repository::dec_use_count(key_t) {}

void assets_repository::emplace(guid guid, const fs::path& path, asset asset) {
  if (auto it = path_index_.find(path); it != path_index_.end()) {
    key_t key = it->second;

    if (storage_[key].guid != guid) {
      id_index_.erase(storage_[key].guid);
      id_index_[guid] = key;
    }

    storage_[key].guid = guid;
    storage_[key].asset = asset;
  } else {
    key_t key = storage_.emplace(std::move(asset), path, guid);
    path_index_[path] = key;
    id_index_[guid] = key;
  }
  logger::core::Info("assets_repository::emplace {0} ({1})", path.c_str(), guid.str());
}

asset_handle assets_repository::load(const fs::path& path) {
  auto it = path_index_.find(path);
  if (it == path_index_.end())
    return {};

  return { this, it->second };
}

asset_handle assets_repository::load(guid guid) {
  auto it = id_index_.find(guid);
  if (it == id_index_.end())
    return {};

  return { this, it->second };
}

asset_handle assets_repository::create_asset(guid guid, const fs::path& path) {
  key_t key = storage_.emplace(asset {}, path, guid);
  path_index_[path] = key;
  id_index_[guid] = key;
  return { this, key };
}

void assets_repository::save_buffer(uint64_t id) {
  fs::path buffer_path = fs::append(fs::temp_directory_path(), std::to_string(id));
  std::ofstream dst(buffer_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
  dst.close();
  buffers().save(id, buffer_path, 0);
}

void assets_filesystem::load_assets(assets_repository* repository) const {
  for (auto it = fs::recursive_directory_iterator(root_); it != fs::recursive_directory_iterator(); it++) {
    if (it->is_directory())
      continue;

    asset asset;
    if (!read(it->path(), asset))
      continue;

    if (!asset.contains("__guid"))
      continue;

    guid id = guid::from_string(asset["__guid"]);
    if (!id.is_valid())
      continue;

    fs::path buffers_directory = get_buffers_path(it->path());
    stream_buffers& buffers = repository->buffers();
    visit_recursive(asset.begin(), asset.end(), [&] (experimental::asset& a) {
      if (a.contains("__binary_hash")) {
        uint32_t buf_hash = a["__binary_hash"];
        fs::path buf_path = fs::append(buffers_directory, std::to_string(buf_hash));
        uint64_t buf_id = buffers.map(buf_path);
        a["__buffer_id"] = buf_id;
        return visit_recursive_result::CONTINUE;
      }

      return visit_recursive_result::RECURSE;
    });

    fs::path relative = fs::relative(it->path(), root_);
    repository->emplace(id, relative, std::move(asset));
  }
}

void assets_filesystem::load(assets_repository* repository, const fs::path& path) const {
  asset asset;
  fs::path fullpath = fs::append(root_, path);
  if (!read(fullpath, asset))
    return;

  if (!asset.contains("__guid"))
    return;

  guid id = guid::from_string(asset["__guid"]);
  if (!id.is_valid())
    return;

  fs::path buffers_directory = get_buffers_path(fullpath);
  stream_buffers& buffers = repository->buffers();
  visit_recursive(asset::iterator { &asset }, asset.end(), [&] (experimental::asset& a) {
    if (a.contains("__binary_hash")) {
      uint32_t buf_hash = a["__binary_hash"];
      fs::path buf_path = fs::append(buffers_directory, std::to_string(buf_hash));
      uint64_t buf_id = buffers.map(buf_path);
      a["__buffer_id"] = buf_id;
      return visit_recursive_result::CONTINUE;
    }

    return visit_recursive_result::RECURSE;
  });

  repository->emplace(id, path, std::move(asset));
}

fs::path assets_filesystem::get_buffers_path(const fs::path& path) {
  return fs::concat(path, ".buffers");
}

void assets_filesystem::save_assets(assets_repository* repository) {
  for (auto [asset, path, guid] : *repository) {
    save(repository, asset, path);
  }
}

void assets_filesystem::save(assets_repository* repository, const fs::path& path) {
  save(repository, *repository->load(path), path);
}

void assets_filesystem::save(assets_repository* repository, const asset& asset, const fs::path& path) {
  fs::path fullpath = fs::append(root_, path);
  stream_buffers& buffers = repository->buffers();

  std::unordered_map<uint32_t, bool> hashes;
  fs::path buffers_directory = get_buffers_path(fullpath);

  visit_recursive(asset.begin(), asset.end(), [&] (const experimental::asset& a) {
    if (a.contains("__buffer_id")) {
      uint32_t buf_id = a["__buffer_id"];
      bool is_loaded = buffers.is_loaded(buf_id);
      uint32_t buf_hash = buffers.hash(buf_id);
      if (!hashes.count(buf_hash)) {
        auto [size, data] = buffers.get(buf_id);

        fs::path buf_path = fs::append(buffers_directory, std::to_string(buf_hash));
        std::ofstream dst(buf_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        std::copy_n(data, size, std::ostreambuf_iterator<char>(dst));

        hashes[buf_hash] = true;
      }

      if (!is_loaded)
        buffers.unload(buf_id);

      return visit_recursive_result::CONTINUE;
    }

    return visit_recursive_result::RECURSE;
  });

  for (auto& [hash, used] : hashes) {
    if (!used) {
      fs::path buf_path = fs::append(buffers_directory, std::to_string(hash));
      fs::remove(buf_path);
    }
  }

  write(fullpath, asset);
}

};
