#pragma once

#include "base/guid.h"
#include "base/json.hpp"
#include "platform/file_system.h"
#include "base/slot_map.h"

#include <map>
#include <sstream>

namespace experimental::assets {

using asset = nlohmann::json;
using buffer_t = std::unique_ptr<std::istream>;

bool read(const fs::path &path, asset &data);
void write(const fs::path &path, const asset &asset);

class repository {
 public:
  struct item {
    guid id;
    fs::path path;
    asset asset;
    uint32_t use_count;
  };

  using container = stdext::slot_map<item>;
  using key = container::key_type;
  using iterator = container::iterator;
  using reference = container::reference;
  using const_reference = container::const_reference;

  key emplace(const guid& id, const fs::path& path) {
    key key = assets_.emplace(item { .id = id, .path = path, .asset = {}, .use_count = 0 });
    id_index_map_[id] = key;
    path_index_map_[path] = key;
    return key;
  }

  void erase(key key) {
    auto iterator = assets_.find(key);
    id_index_map_.erase(iterator->id);
    path_index_map_.erase(iterator->path);
    assets_.erase(iterator);
  }

  iterator find(key key) {
    return assets_.find(key);
  }

  std::pair<key, bool> find(const guid& id) {
    auto index_iterator = id_index_map_.find(id);
    if (index_iterator == id_index_map_.end())
      return { {}, false };

    return { index_iterator->second, true };
  }

  std::pair<key, bool> find(const fs::path& path) {
    auto index_iterator = path_index_map_.find(path);
    if (index_iterator == path_index_map_.end())
      return { {}, false };

    return { index_iterator->second, true };
  }

  constexpr reference operator[](const key& key)              { return assets_[key]; }
  constexpr const_reference operator[](const key& key) const  { return assets_[key]; }

  iterator begin() { return assets_.begin(); }
  iterator end() { return assets_.end(); }

 private:
  container assets_;
  std::unordered_map<guid, key> id_index_map_;
  std::unordered_map<std::string, key> path_index_map_;
};

class provider {
 public:
  virtual fs::path get_path(const guid& id) = 0;
  virtual asset load(const fs::path&) = 0;
  virtual void load_buffer(const fs::path&, uint64_t buffer_id, std::ostream&) = 0;
  virtual void save(const fs::path&, const asset&) = 0;
//  virtual void save_buffer(const fs::path&, uint64_t buffer_id, const std::istream&) = 0;
  virtual void reload(repository&) = 0;
};

class filesystem_provider : public provider {
 public:
  void add(const fs::path&);

  asset load(const fs::path&) override;
  fs::path get_path(const guid& id) override;
  void load_buffer(const fs::path&, uint64_t buffer_id, std::ostream&) override;
  void save(const fs::path&, const asset&) override;
  void reload(repository&) override;

 private:
  std::unordered_map<guid, std::string> guid_to_path_;
};

class handle {
 public:
  handle(repository*, const repository::key&);
  ~handle();

  handle(const handle&);
  handle(handle&&) noexcept;

  handle& operator=(const handle&);
  handle& operator=(handle&&) noexcept;

  asset* operator->() { return &get().asset; }
  asset& operator*() { return get().asset; }

  const asset* operator->() const { return &get().asset; }
  const asset& operator*() const { return get().asset; }

  explicit operator bool() const noexcept { return repository_ != nullptr && repository_->find(key_) != repository_->end(); }

  [[nodiscard]] const guid& id() const { return get().id; }
  [[nodiscard]] const fs::path& path() const { return get().path; }

 private:
  repository::reference get() { return (*repository_)[key_]; }
  [[nodiscard]] repository::const_reference get() const { return (*repository_)[key_]; }

  void swap(handle& other) noexcept;

 private:
  repository* repository_;
  repository::key key_;
};

class registry1 {
 public:
  handle load(provider*, const guid& id);
  handle load(provider*, const fs::path& path);
  buffer_t load_buffer(provider*, const handle& handle, uint64_t buffer_id);

 private:
  repository repository_;
};

class registry {
 public:
  void init(provider* provider) {
    provider_ = provider;
  }

  handle load(const guid& id) {
    if (auto [ key, success ] = repository_.find(id); success)
      return { &repository_, key };

    fs::path path = provider_->get_path(id);
    repository::key key = repository_.emplace(id, path);
    repository_[key].asset = provider_->load(path);
    return { &repository_, key };
  }

  handle load(const fs::path& path) {
    if (auto [ key, success ] = repository_.find(path); success)
      return { &repository_, key };

    asset asset = provider_->load(path);
    repository::key key = repository_.emplace(guid::from_string(asset["__guid"]), path);
    repository_[key].asset = std::move(asset);
    return { &repository_, key };
  }

  buffer_t load_buffer(const handle& handle, uint64_t buffer_id) {
    if (!handle)
      return { };

    std::stringstream stream;
    provider_->load_buffer(handle.path(), buffer_id, stream);
    return std::make_unique<std::stringstream>(std::move(stream));
  }

  void save_changes(const handle& handle) {
    provider_->save(handle.path(), *handle);
  }

  void reload() {
    provider_->reload(repository_);
  }

 private:
  repository repository_;
  provider* provider_ = nullptr;
};


}
