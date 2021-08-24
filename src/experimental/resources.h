#pragma once

#include "platform/file_system.h"
#include "base/guid.h"
#include "base/slot_map.h"
#include "core/meta/type.h"
#include "assets.h"

#include <unordered_map>
#include <utility>

namespace experimental::resources {

class pool_base {
 protected:
  template<class F>
  using container_base = stdext::slot_map<F>;

 public:
  using key = typename container_base<struct _>::key_type;
};

template<class T>
using load_func_t = std::unique_ptr<T> (*)(std::istream& stream);

template<class T>
class pool : public pool_base {
 private:
  struct info {
    guid id = {};
    fs::path path = {};
    std::unique_ptr<T> ptr = {};
    uint32_t use_count = {};
  };

  using container = pool_base::container_base<info>;

 public:
  explicit pool(load_func_t<T> load_func) : load_func_(load_func) {}

  key load(const fs::path& path, experimental::assets::registry* assets_registry) {
    if (auto it = path_to_keys_.find(path); it != path_to_keys_.end()) {
      return it->second;
    }

    fs::path meta_path = fs::append(path, ".meta");
    experimental::assets::handle handle = assets_registry->load(meta_path);
    experimental::assets::buffer_t buffer = assets_registry->load_buffer(handle, handle->at("data"));

    key key = table_.insert(
        {
          .ptr = load_func_(*buffer),
          .id = handle.id(),
          .path = path,
          .use_count = 0
        });

    id_to_keys_.insert({handle.id(), key});
    path_to_keys_.insert({path.c_str(), key});
    return key;
  }

  key load(const guid& id, experimental::assets::registry* assets_registry) {
    if (auto it = id_to_keys_.find(id); it != id_to_keys_.end()) {
      return it->second;
    }

    experimental::assets::handle handle = assets_registry->load(id);
    experimental::assets::buffer_t buffer = assets_registry->load_buffer(handle, handle->at("data"));

    key key = table_.insert(
        {
            .ptr = load_func_(*buffer),
            .id = id,
            .path = handle.path(),
            .use_count = 0
        });

    id_to_keys_.insert({id, key});
    path_to_keys_.insert({handle.path().c_str(), key});
    return key;
  }

  T *get(key key) { return table_[key].ptr.get(); }
  [[nodiscard]] const T *get(key key) const { return table_[key].ptr.get(); }

  void inc_use_count(key key) {
    table_[key].use_count++;
  }

  void dec_use_count(key key) {
    assert(table_[key].use_count > 0);

    uint32_t count = --table_[key].use_count;
    if (count == 0) {
      table_.erase(key);
      path_to_keys_.erase(table_[key].path.string());
      id_to_keys_.erase(table_[key].id);
    }
  }

 private:
  container table_ = {};
  std::unordered_map<std::string, key> path_to_keys_ = {};
  std::unordered_map<guid, key> id_to_keys_ = {};
  load_func_t<T> load_func_;
};

template<class T>
class handle {
 private:
  using pool = pool<T>;
  using key = typename pool_base::key;

 public:
  handle(const handle& other) : pool_(other.pool_), key_(other.key_) {
    if (pool_)
      pool_->inc_use_count(key_);
  }

  handle(handle&& other) noexcept : pool_(other.pool_), key_(other.key_) {
    other.release();
  }

  ~handle() { reset(); }

  handle& operator=(const handle& other) {
    handle(other).swap(*this);
    return *this;
  }

  handle& operator=(handle&& other) noexcept {
    handle(std::move(other)).swap(*this);
    return *this;
  }

  T* operator->() { return get(); }
  T& operator*() { return *get(); }

  const T* operator->() const { return get(); }
  const T& operator*() const { return *get(); }

  explicit operator bool() const noexcept { return pool_ != nullptr; }

  T* get() { return pool_->get(key_); }
  const T* get() const { return pool_->get(key_); }

  void reset() {
    if (pool_)
      pool_->dec_use_count(key_);
    release();
  }

 private:
  handle(pool* pool, key key) : pool_(pool), key_(key) {}

  void release() {
    pool_ = nullptr;
  }

  void swap(handle& other) noexcept {
    std::swap(pool_, other.pool_);
    std::swap(key_, other.key_);
  }

 private:
  pool* pool_;
  key key_;
};

class registry {
 private:
  using key = stdext::slot_map<struct _>::key_type;

  template<class T>
  struct key_storage {
    static key key;
  };

  template<class T>
  pool<T>* get_pool() {
    std::unique_ptr<pool_base> ptr = pools_[key_storage<T>::key];
    return static_cast<pool<T>*>(ptr.get());
  }

 public:
  void init(experimental::assets::registry* assets_registry) {
    assets_registry_ = assets_registry;
  }

  template<class T>
  handle<T> load(const fs::path& path) {
    pool<T>* pool_ptr = get_pool<T>();
    return { pool_ptr, pool_ptr->load(path, assets_registry_)};
  }

  template<class T>
  handle<T> load(const guid& id) {
    pool<T>* pool_ptr = get_pool<T>();
    return { pool_ptr, pool_ptr->load(id, assets_registry_)};
  }

  template<class T>
  void register_pool(load_func_t<T> load_func) {
    key_storage<T>::key = pools_.insert(std::make_unique<pool<T>>(load_func));
  }

  template<class T>
  void unregister_pool() {
    pools_.erase(key_storage<T>::key);
  }

 private:
  stdext::slot_map<std::unique_ptr<pool_base>> pools_;
  experimental::assets::registry* assets_registry_;
};

class compiler {
  using compile_func_t = bool(*)(std::ifstream& stream, const experimental::assets::asset& meta, std::ostream&);

  void register_compiler(const std::string& type_name, compile_func_t compile_func) {
    compilers_.emplace(std::make_pair(type_name, compile_func));
  }

  void unregister_compiler(const std::string& type_name) {
    compilers_.erase(type_name);
  }

  bool compile(const fs::path& path, assets::registry* registry) {
    assets::handle meta = registry->load(path);

    auto it = compilers_.find(meta->at("type"));
    if (it == compilers_.end())
      return false;

    compile_func_t compile_func = it->second;
    std::ifstream file = fs::read_file(meta->at("source_path"));



  }

 private:
  std::unordered_map<std::string, compile_func_t> compilers_;

};

}


