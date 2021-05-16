#pragma once

#include "base/slot_map.h"
#include "base/log.h"

#include <unordered_map>
#include <sstream>
#include <filesystem>

namespace assets {

template<class T>
std::unique_ptr<T> load_asset(const std::istream&);

namespace details {

static bool read_file(const std::filesystem::path& path, std::ostream& stream);

template<class T>
class registry {
 private:
  using container = stdext::slot_map<std::pair<std::unique_ptr<T>, uint32_t>>;

 public:
  using key = typename container::key_type;

  key load(const std::string &path) {
    if (auto it = path_to_keys_.find(path); it != path_to_keys_.end()) {
      return it->second;
    }

    std::stringstream stream;
    read_file(path, stream);

    key key = table_.insert({ ::assets::load_asset<T>(stream), 0 });
    path_to_keys_.insert({path, key});
    return key;
  }

  T *get(key key) { return table_[key].first.get(); }
  const T *get(key key) const { return table_[key].first.get(); }

  void inc_ref_count(key key) { table_[key].second++; }
  void dec_ref_count(key key) {
    assert(table_[key].second > 0);

    uint32_t count = --table_[key].second;
    if (count == 0) {
      table_.erase(key);

      auto it = std::find_if(path_to_keys_.begin(), path_to_keys_.end(), [&](const auto& pair){
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
      registry_->inc_ref_count(key_);
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

  T* get() { return registry_->get(key_); }
  const T* get() const { return registry_->get(key_); }

  void reset() {
    if (registry_)
      registry_->dec_ref_count(key_);
    release();
  }

 private:
  void release() {
    registry_ = nullptr;
  }

  void swap(handle& other) noexcept {
    std::swap(registry_, other.registry_);
    std::swap(key_, other.key_);
  }

  explicit handle(registry* registry, key key)
      : registry_(registry)
      , key_(key)
  {
    if (registry_)
      registry_->inc_ref_count(key_);
  }

  friend handle load(const std::string& path);

 private:
  registry* registry_;
  key key_;
};

template<class T>
handle<T> load(const std::string& path) {
  details::registry<T> reg = details::get_registry<T>();
  return { &reg, reg.load(path) };
}

}



