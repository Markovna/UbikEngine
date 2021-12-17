#pragma once

#include <unordered_map>
#include <iostream>
#include <vector>
#include "core/meta/type.h"
#include "core/meta/registration.h"
#include "base/iterator_range.h"

class systems_registry {
 public:
  static const std::shared_ptr<void>& invalid() {
    static std::shared_ptr<void> invalid;
    return invalid;
  };

  template<class T>
  class handle {
   public:
    explicit handle() noexcept : ptr_(&invalid()) {}
    explicit handle(const std::shared_ptr<void>& ptr) noexcept : ptr_(&ptr) {}

    T& operator*() { return *get(); }
    const T& operator*() const { return *get(); }

    T* operator->() { return get(); }
    const T* operator->() const { return get(); }

    T* get() { return static_cast<T*>(ptr_->get()); }
    const T* get() const { return static_cast<T*>(ptr_->get()); }

    explicit operator bool() const { return (bool) *ptr_; }

   private:
    const std::shared_ptr<void>* ptr_;
  };

 private:
  struct pool_base {
    virtual ~pool_base() = default;
  };

  template<class T>
  struct pool : public pool_base {
    std::vector<std::unique_ptr<T>> entries;
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
    singletons_[type_id] = std::move(ptr);
    return handle<T> { singletons_[type_id] };
  }

  template<class T>
  void remove() {
    auto it = singletons_.find(meta::get_typeid<T>());
    if (it != singletons_.end()) {
      it->second.reset();
    }
  }

  template<class T>
  handle<T> get() {
    auto type_id = meta::get_typeid<T>();
    return handle<T> { singletons_[type_id] };
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
  [[nodiscard]] auto view() {
    auto pool_ptr = reinterpret_cast<pool<T>*>(systems_[meta::get_typeid<T>()].get());
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
  std::unordered_map<meta::typeid_t, std::shared_ptr<void>> singletons_;
  std::unordered_map<meta::typeid_t, std::unique_ptr<pool_base>> systems_;
};

template<class T>
using system_ptr = systems_registry::handle<T>;