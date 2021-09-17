#pragma once

#include "base/guid.h"
#include "base/json.hpp"
#include "base/slot_map.h"
#include "core/assets/asset.h"
#include "platform/file_system.h"

#include <map>
#include <sstream>
#include <random>

namespace assets {

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

  virtual ~provider() = default;
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

handle load(provider* provider, repository& rep, const guid& id);
handle load(provider* provider, repository& rep, const fs::path& path);

handle load(provider* provider, const guid& id);
handle load(provider* provider, const fs::path& path);
buffer_t load_buffer(provider* provider, const handle& handle, const char* name);

uint64_t add_buffer(handle& handle, const char* name);

extern repository* g_repository;
void init();
void shutdown();

}


