#pragma once

#include "platform/file_system.h"
#include "base/guid.h"
#include "base/slot_map.h"
#include "core/meta/type.h"
#include "assets.h"

#include <unordered_map>
#include <utility>

namespace resources {

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

  std::pair<bool, key> load(const fs::path& path, assets::provider* provider) {
    if (auto it = path_to_keys_.find(path); it != path_to_keys_.end()) {
      return { true, it->second};
    }

    fs::path meta_path = fs::concat(path, ".meta");
    assets::handle handle = assets::load(provider, meta_path);
    if (!handle) {
      logger::core::Error("Couldn't load resource at path {0}", path.c_str());
      return { false, {} };
    }

    assets::buffer_t buffer = assets::load_buffer(provider, handle, "data");
    if (!buffer) {
      return { false, {} };
    }

    std::unique_ptr<T> ptr = load_func_(*buffer);
    if (!ptr) {
      return { false, {} };
    }

    key key = table_.insert(
        {
          .ptr = std::move(ptr),
          .id = handle.id(),
          .path = path,
          .use_count = 0
        });


    id_to_keys_.insert({handle.id(), key});
    path_to_keys_.insert({path.c_str(), key});
    return { true, key };
  }

  std::pair<bool, key> load(const guid& id, assets::provider* provider) {
    if (auto it = id_to_keys_.find(id); it != id_to_keys_.end()) {
      return { true, it->second };
    }

    assets::handle handle = assets::load(provider, id);
    if (!handle) {
      logger::core::Error("Couldn't load resource with id {0}", id.str());
      return { false, {} };
    }

    assets::buffer_t buffer = assets::load_buffer(provider, handle, "data");
    if (!buffer) {
      return { false, {} };
    }

    std::unique_ptr<T> ptr = load_func_(*buffer);
    if (!ptr) {
      return { false, {} };
    }

    key key = table_.insert(
        {
            .ptr = std::move(ptr),
            .id = id,
            .path = handle.path(),
            .use_count = 0
        });

    id_to_keys_.insert({id, key});
    path_to_keys_.insert({handle.path().c_str(), key});
    return { true, key };
  }

  const guid& get_id(key key) const { return table_[key].id; }
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
  handle() : pool_(nullptr) {}

  handle(const handle& other) : pool_(other.pool_), key_(other.key_) {
    if (pool_)
      pool_->inc_use_count(key_);
  }

  handle(handle&& other) noexcept : pool_(other.pool_), key_(other.key_) {
    other.release();
  }

  handle(pool* pool, key key) : pool_(pool), key_(key) {
    if (pool_)
      pool_->inc_use_count(key_);
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

  [[nodiscard]] const guid& id() const { return pool_->get_id(key_); }

  void reset() {
    if (pool_)
      pool_->dec_use_count(key_);
    release();
  }

 private:

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

template<class T>
struct pool_storage {
  std::unique_ptr<pool<T>> pool;
};

template<class T>
pool_storage<T>& get_pool_storage() {
  static pool_storage<T> storage;
  return storage;
}

template<class T>
void register_pool(load_func_t<T> load_func) {
  get_pool_storage<T>().pool = std::make_unique<pool<T>>(load_func);
}

template<class T>
void unregister_pool() {
  get_pool_storage<T>().pool.reset();
}

template<class T>
handle<T> load(const fs::path& path, assets::provider* provider) {
  pool<T>* pool_ptr = get_pool_storage<T>().pool.get();
  auto result = pool_ptr->load(path, provider);
  if (!result.first)
    return {};

  return { pool_ptr, result.second};
}

template<class T>
handle<T> load(const guid& id, assets::provider* provider) {
  pool<T>* pool_ptr = get_pool_storage<T>().pool.get();
  auto result = pool_ptr->load(id, provider);
  if (!result.first)
    return {};

  return { pool_ptr, result.second};
}


void init();
void shutdown();

class compiler {
 public:
  using compile_func_t = bool(*)(std::ifstream& stream, const asset& meta, std::ostream&);

  void register_compiler(const std::string& type_name, compile_func_t compile_func) {
    compilers_.emplace(std::make_pair(type_name, compile_func));
  }

  void unregister_compiler(const std::string& type_name) {
    compilers_.erase(type_name);
  }

  bool compile(const fs::path& path, assets::provider* provider) {
    assets::handle meta = assets::load(provider, path);

    auto it = compilers_.find(meta->at("type"));
    if (it == compilers_.end())
      return false;

    // remove old buffer if exists
    if (meta->contains("data")) {
      provider->remove_buffer(path, meta->at("data"));
    }

    compile_func_t compile_func = it->second;
    std::stringstream out;
    std::ifstream file = fs::read_file(fs::absolute(meta->at("source_path")));

    bool success = compile_func(file, *meta, out);
    if (success) {
      uint64_t buffer_id = assets::add_buffer(meta, "data");
      provider->save_buffer(path, buffer_id, out);
      provider->save(path, *meta);
      return true;
    }

    return false;
  }

 private:
  std::unordered_map<std::string, compile_func_t> compilers_;
};

extern compiler* g_compiler;
void init_compiler();
void shutdown_compiler();
void compile_all_assets(const char *directory, assets::provider* provider);

template<class T>
handle<T> resolve(const asset& asset, const char* key, assets::provider* provider) {
  std::string id;
  ::assets::get(asset, key, id);
  return load<T>(guid::from_string(id.c_str()), provider);
}

}


