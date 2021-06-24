#pragma once

#include "base/log.h"
#include "base/guid.h"
#include "base/slot_map.h"
#include "asset_loader.h"
#include "asset.h"
#include "platform/file_system.h"

#include <unordered_map>
#include <fstream>
#include <sstream>
#include <filesystem>

class texture;
class shader;

namespace assets {

namespace details {

template<class T>
class registry {
 private:
  struct info {
    guid id{};
    std::unique_ptr<T> ptr{};
    uint32_t use_count{};
  };

  using container = stdext::slot_map<info>;

 public:
  using key = typename container::key_type;

//  key load(guid id) {
//    if (auto it = id_to_keys_.find(id); it != id_to_keys_.end()) {
//      return it->second;
//    }
//
//    fs::path path { id.str() };
//    path.replace_extension(".import");
//
//    fs::paths::import()
//
//  }

  guid get_guid(const fs::path &path) {
    asset meta = assets::read(fs::concat(path, ".meta"));
    std::string guid;
    assets::get(meta, "__guid", guid);
    return guid::from_string(guid);
  }

  key load(const fs::path &path) {
    if (auto it = path_to_keys_.find(path); it != path_to_keys_.end()) {
      return it->second;
    }

    fs::path absolute_path { fs::absolute(path) };

    guid guid = get_guid(absolute_path);
    fs::path asset_path { fs::append(fs::paths::import(), guid.str()) };

    std::ifstream stream = fs::read_file(asset_path);
    key key {
      table_.insert({
        .ptr = ::assets::loader::load<T>(stream),
        .id = guid,
        .use_count = 0
      })
    };

    id_to_keys_.insert({guid, key});
    path_to_keys_.insert({path.c_str(), key});
    return key;
  }

  guid id(key key) const {
    if (auto it = table_.find(key); it != table_.end()) {
      return it->second.id;
    }
    return guid::invalid();
  }

  T *get(key key) { return table_[key].ptr.get(); }
  [[nodiscard]] const T *get(key key) const { return table_[key].ptr.get(); }

  void inc_use_count(key key) { table_[key].use_count++; }
  void dec_use_count(key key) {
    assert(table_[key].use_count > 0);

    uint32_t count = --table_[key].use_count;
    if (count == 0) {
      table_.erase(key);

      auto it = std::find_if(path_to_keys_.begin(), path_to_keys_.end(), [&](const auto &pair) {
        return pair.second == key;
      });

      if (it != path_to_keys_.end()) {
        path_to_keys_.erase(it);
      }
    }
  }

 private:
  container table_ = {};
  std::unordered_map<std::string, key> path_to_keys_ = {};
  std::unordered_map<guid, key> id_to_keys_ = {};
};

template<class T>
registry<T>* get_registry() {
  static registry<T> inst;
  return &inst;
}

}

template<class T>
class handle {
 private:
  using registry = details::registry<T>;
  using key = typename details::registry<T>::key;

 public:
  handle()
    : registry_(nullptr)
    , key_()
  {}

  ~handle() { reset(); }

  handle(const handle& other)
    : registry_(other.registry)
    , key_(other.key)
  {
    if (registry_)
      registry_->inc_use_count(key_);
  }

  handle(handle&& other) noexcept
    : registry_(other.registry_)
    , key_(other.key_)
  {
    other.release();
  }

  handle& operator=(const handle& other) {
    handle(other).swap(*this);
    return *this;
  }

  handle& operator=(handle&& other) noexcept {
    handle(std::move(other)).swap(*this);
    return *this;
  }

  explicit operator bool() const noexcept { return registry_ != nullptr; }

  T* operator->() { return get(); }
  T& operator*() { return *get(); }

  const T* operator->() const { return get(); }
  const T& operator*() const { return *get(); }

  [[nodiscard]] guid id() const { return registry_->id(key_); }

  T* get() { return registry_->get(key_); }
  const T* get() const { return registry_->get(key_); }

  void reset() {
    if (registry_)
      registry_->dec_use_count(key_);
    release();
  }

 private:
  handle(registry* registry, key key)
      : registry_(registry)
      , key_(key)
  {
    if (registry_)
      registry_->inc_use_count(key_);
  }

  void release() {
    registry_ = nullptr;
  }

  void swap(handle& other) noexcept {
    std::swap(registry_, other.registry_);
    std::swap(key_, other.key_);
  }

  template<class A>
  friend handle<A> load(const char* path);

 public:
  registry* registry_;
  key key_;
};

void init();

template<class T>
handle<T> load(const char* path) {
  details::registry<T>* reg = details::get_registry<T>();
  return { reg, reg->load(path) };
}

template<class T>
handle<T> load(guid id);

}

using texture_handle = assets::handle<texture>;
using shader_handle = assets::handle<shader>;





