#pragma once

#include "base/log.h"
#include "base/slot_map.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

struct engine;

struct vtable {
  using invoke_ptr_t  = void (*)(void* const, engine*);
  using destroy_ptr_t = void (*)(void* const);

  invoke_ptr_t start_ptr;
  invoke_ptr_t update_ptr;
  invoke_ptr_t stop_ptr;
  destroy_ptr_t destroy_ptr;

  vtable(const vtable&) = delete;
  vtable(vtable&&) = delete;

  vtable& operator= (const vtable&) = delete;
  vtable& operator= (vtable&&) = delete;

  ~vtable() = default;
};

class plugin {
 public:
  plugin(void* ptr, const vtable* vtable_ptr) : ptr_(ptr), vtable_ptr_(vtable_ptr) {}

  plugin(const plugin&) = delete;
  plugin& operator=(const plugin&) = delete;

  plugin(plugin&& other) noexcept : ptr_(other.ptr_), vtable_ptr_(other.vtable_ptr_) {
    other.ptr_ = nullptr;
    other.vtable_ptr_ = nullptr;
  }

  plugin& operator=(plugin&& other) noexcept {
    plugin(std::move(other)).swap(*this);
    return *this;
  }

  ~plugin() {
    if (ptr_) {
      vtable_ptr_->destroy_ptr(ptr_);
      ptr_ = nullptr;
    }
  }

  void update(engine* e) {
    vtable_ptr_->update_ptr(ptr_, e);
  }

  void start(engine* e) {
    vtable_ptr_->start_ptr(ptr_, e);
  }

  void stop(engine* e) {
    vtable_ptr_->stop_ptr(ptr_, e);
  }

  [[nodiscard]] void* raw() const {
    return ptr_;
  }

 private:
  void swap(plugin& other) noexcept {
    std::swap(ptr_, other.ptr_);
    std::swap(vtable_ptr_, other.vtable_ptr_);
  }

 private:
  void* ptr_;
  const vtable* vtable_ptr_;
};

namespace dispatcher {

namespace detail {

template <class Default, class AlwaysVoid, template<class...> class Op, class... Args>
struct detector {
  using value_t = std::false_type;
  using type = Default;
};

template <class Default, template<class...> class Op, class... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
  using value_t = std::true_type;
  using type = Op<Args...>;
};

} // namespace detail

struct nonesuch {
  ~nonesuch() = delete;
  nonesuch(nonesuch const&) = delete;
  void operator=(nonesuch const&) = delete;
};

template <template<class...> class Op, class... Args>
using is_detected = typename detail::detector<nonesuch, void, Op, Args...>::value_t;

template<class T> using update_detector = decltype(std::declval<T&>().update(nullptr));
template<class T> using has_update = is_detected<update_detector, T>;
template<class T> constexpr bool has_update_v = has_update<T>::value;
template<class T> using has_update_t = typename has_update<T>::type;

template<class T> using start_detector = decltype(std::declval<T&>().start(nullptr));
template<class T> using has_start = is_detected<start_detector, T>;
template<class T> constexpr bool has_start_v = has_start<T>::value;
template<class T> using has_start_t = typename has_start<T>::type;

template<class T> using stop_detector = decltype(std::declval<T&>().stop(nullptr));
template<class T> using has_stop = is_detected<stop_detector, T>;
template<class T> constexpr bool has_stop_v = has_stop<T>::value;
template<class T> using has_stop_t = typename has_stop<T>::type;


template<class T>
void invoke_start_impl(void* const ptr, engine* e, std::true_type) {
  static_cast<T*>(ptr)->start(e);
}

template<class T>
void invoke_start_impl(void* const, engine*, std::false_type) {}

template<class T>
void invoke_update_impl(void* const ptr, engine* e, std::true_type) {
  static_cast<T*>(ptr)->update(e);
}

template<class T>
void invoke_update_impl(void* const, engine*, std::false_type) {}

template<class T>
void invoke_stop_impl(void* const ptr, engine* e, std::true_type) {
  static_cast<T*>(ptr)->stop(e);
}

template<class T>
void invoke_stop_impl(void* const, engine*, std::false_type) {}

template<class T>
void invoke_start(void* const ptr, engine* e) {
  invoke_start_impl<T>(ptr, e, has_start_t<T>());
}

template<class T>
void invoke_update(void* const ptr, engine* e) {
  invoke_update_impl<T>(ptr, e, has_update_t<T>());
}

template<class T>
void invoke_stop(void* const ptr, engine* e) {
  invoke_stop_impl<T>(ptr, e, has_stop_t<T>());
}

template<class T>
void destroy(void* const ptr) {
  delete static_cast<T*>(ptr);
}

};

class plugins_registry {
 public:
  using container = stdext::slot_map<plugin>;
  using iterator = container::iterator;
  using key = container::key_type;

 public:
  template<class T, class ...Args>
  T* add_plugin(const char* name, Args... args) {
    assert(names_map_.count(std::string(name)) == 0 && "Plugin has been already added");

    logger::core::Info("plugins_registry::add_plugin {}", name);

    key key = plugins_.emplace(plugin { new std::decay_t<T> (std::forward<Args>(args)...), get_vtable<T>() });
    names_map_[std::string(name)] = key;
    return static_cast<T*>(plugins_[key].raw());
  }

  void remove_plugin(const char* name);

  template<class T>
  T* get_plugin(const char* name) {
    key key = names_map_[std::string(name)];
    return static_cast<T*>(plugins_[key].raw());
  }

  iterator begin() { return plugins_.begin(); }
  iterator end() { return plugins_.end(); }

 private:
  template<class T>
  static const vtable* get_vtable() {
    using F = std::decay_t<T>;
    static const vtable vt {
        dispatcher::invoke_start<F>,
        dispatcher::invoke_update<F>,
        dispatcher::invoke_stop<F>,
        dispatcher::destroy<F>
    };
    return &vt;
  }

 private:
  container plugins_;
  std::unordered_map<std::string, key> names_map_;
};

