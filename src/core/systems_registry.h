#pragma once

#include <unordered_map>
#include <iostream>
#include <vector>
#include "core/meta/type.h"
#include "core/meta/registration.h"
#include "base/iterator_range.h"

class systems_registry {
 private:
  struct singleton_base {
    virtual ~singleton_base() = default;
  };

  template<class T>
  struct singleton : public singleton_base {
    std::unique_ptr<T> ptr;
  };

  struct pool_base {
    virtual ~pool_base() = default;
  };

  template<class T>
  struct pool : public pool_base {
    std::vector<std::unique_ptr<T>> entries;
  };

 public:
  template<class T>
  class handle {
   public:
    explicit handle() noexcept : singleton_(nullptr) {}
    explicit handle(const singleton<T>* s) noexcept : singleton_(s) {}

    T& operator*() { return *get(); }
    const T& operator*() const { return *get(); }

    T* operator->() { return get(); }
    const T* operator->() const { return get(); }

    T* get() { return static_cast<T*>(singleton_->ptr.get()); }
    const T* get() const { return static_cast<T*>(singleton_->ptr.get()); }

    explicit operator bool() const { return (bool) singleton_->ptr; }

   private:
    const singleton<T>* singleton_;
  };

 public:
  template<class T>
  class pool_view {
   public:
    explicit pool_view(pool<T>* ptr) : pool_ptr_(ptr) {}

    auto begin() { return pool_ptr_->entries.begin(); }
    auto end() { return pool_ptr_->entries.end(); }

   private:
    pool<T>* pool_ptr_;
  };

 public:
  template<class T, class F>
  handle<T> set(std::unique_ptr<F> ptr) {
    auto type_id = meta::get_typeid<T>();
    auto s = std::make_unique<singleton<F>>();
    s->ptr = std::move(ptr);
    singletons_[type_id] = std::move(s);
    return handle<T> { static_cast<singleton<T>*>(singletons_[type_id].get()) };
  }

  template<class T>
  void remove() {
    auto it = singletons_.find(meta::get_typeid<T>());
    if (it != singletons_.end()) {
      auto* s = static_cast<singleton<T>*>(it->second.get());
      s->ptr.reset();
    }
  }

  template<class T>
  handle<T> get() const {
    auto type_id = meta::get_typeid<T>();
    return handle<T> { static_cast<singleton<T>*>(singletons_.at(type_id).get()) };
  }


  template<class T, class F>
  F* add(std::unique_ptr<F> system) {
    if (!systems_.count(meta::get_typeid<T>())) {
      systems_[meta::get_typeid<T>()] = std::make_unique<pool<T>>();
    }

    F* ptr = system.get();
    auto pool_ptr = reinterpret_cast<pool<T>*>(systems_[meta::get_typeid<T>()].get());
    pool_ptr->entries.push_back(std::move(system));
    return ptr;
  }

  template<class T, class F>
  void erase(F* system) {
    auto pool_ptr = reinterpret_cast<pool<T>*>(systems_[meta::get_typeid<T>()].get());
    auto it = std::find_if(
        pool_ptr->entries.begin(),
        pool_ptr->entries.end(),
        [&] (auto& ptr) { return ptr.get() == system; }
      );
    if (it != pool_ptr->entries.end()) {
      pool_ptr->entries.erase(it);
    }
  }

  template<class T>
  void erase_all() {
    systems_.erase(meta::get_typeid<T>());
  }

  template<class T>
  [[nodiscard]] auto view() const {
    auto pool_ptr = reinterpret_cast<pool<T>*>(systems_.at(meta::get_typeid<T>()).get());
    return pool_view<T> { pool_ptr };
  }

  template<class T>
  T* first() {
    auto pool_ptr = reinterpret_cast<pool<T>*>(systems_[meta::get_typeid<T>()].get());
    return pool_ptr->entries.front();
  }

  template<class T>
  T* single() {
    auto pool_ptr = reinterpret_cast<pool<T>*>(systems_[meta::get_typeid<T>()].get());
    assert(pool_ptr->entries.size() == 1);
    return pool_ptr->entries.front();
  }

  template<class T>
  size_t size() const {
    auto pool_ptr = reinterpret_cast<pool<T>*>(systems_.at(meta::get_typeid<T>()).get());
    return pool_ptr->entries.size();
  }

 private:
  std::unordered_map<meta::typeid_t, std::unique_ptr<singleton_base>> singletons_;
  std::unordered_map<meta::typeid_t, std::unique_ptr<pool_base>> systems_;
};

template<class T>
using system_ptr = systems_registry::handle<T>;