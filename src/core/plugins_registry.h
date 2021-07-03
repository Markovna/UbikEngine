#pragma once

#include "base/log.h"
#include "base/slot_map.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>


class base_plugin {};

template<class T>
class plugin : public base_plugin, public T {
 public:
  using plugin_type = T;
};

template<class T>
struct plugin_type {
  using type = typename T::plugin_type;
};

template<class T>
using plugin_type_t = typename plugin_type<T>::type;

class plugins_registry {


  class type_info
  {
    static size_t counter() {
      static size_t value{};
      return value++;
    }

   public:
    template<class T>
    static size_t index() {
      static size_t id = counter();
      return id;
    }
  };

  struct plugin_info {
    std::string name;
    std::unique_ptr<base_plugin> plugin_ptr;
  };

  using plugins_pool = std::vector<plugin_info>;

  template<class T, class Iterator>
  class view_base {
   public:
    class iterator_base {
     public:
      using value_type = T;
      using pointer = value_type*;
      using reference = value_type&;

     private:
      using plugin_type = plugin<T>;

     public:
      explicit iterator_base(Iterator curr) : curr_(curr) {}

      iterator_base& operator++() { ++curr_; return *this; }
      iterator_base operator++(int) {
        iterator_base orig = *this;
        return ++(*this), orig;
      }

      [[nodiscard]] bool operator==(const iterator_base& other) const { return curr_ == other.curr_; }
      [[nodiscard]] bool operator!=(const iterator_base& other) const { return !(*this == other); }

      [[nodiscard]] pointer operator->() const {
        return static_cast<pointer>(static_cast<plugin_type*>(curr_->plugin_ptr.get()));
      }

      [[nodiscard]] reference operator*() const {
        return *static_cast<pointer>(static_cast<plugin_type*>(curr_->plugin_ptr.get()));
      }

     private:
      Iterator curr_;
    };

    view_base(Iterator begin, Iterator end)
      : begin_(begin), end_(end) {}

    iterator_base begin() const { return iterator_base(begin_); }
    iterator_base end() const { return iterator_base(end_); }

   private:
    Iterator begin_, end_;
  };

  template<class T>
  plugins_pool& assure() {
    size_t index = type_info::index<T>();

    if (index >= pools_.size()) {
      pools_.resize(index+1);
    }

    plugins_pool& pool = pools_[index];

    return pool;
  }

 public:

  template<class T>
  using view_t = view_base<T, plugins_pool::iterator>;

 public:
  template<class T, class ...Args>
  T* add(const char* name, Args... args) {
    using plugin_type = plugin_type_t<T>;

    plugins_pool& pool = assure<plugin_type>();
    pool.push_back({ name, std::make_unique<T>(args...) });

    names_map_[name] = { type_info::index<plugin_type>(), pool.size() - 1 };
    return static_cast<T*>(pool.back().plugin_ptr.get());
  }

  template<class T>
  T* get(const char* name) {
    if (auto it = names_map_.find(std::string(name)); it != names_map_.end()) {
      auto& [pool_index, index] = it->second;
      static_cast<T*>(pools_[pool_index][index].plugin_ptr.get());
    }

    return nullptr;
  }

  void remove(const char* name) {
    if (auto it = names_map_.find(std::string(name)); it != names_map_.end()) {
      auto& [pool_index, index] = it->second;
      plugins_pool& pool = pools_[pool_index];
      plugin_info& back = pool.back();

      names_map_[back.name].second = index;
      std::swap(pool[index], back);
      pool.pop_back();

      names_map_.erase(name);
    }
  }

  template<class T>
  view_t<T> view() {
    plugins_pool& pool = assure<T>();
    return { pool.begin(), pool.end() };
  }

  template<class T>
  view_t<T> view() const {
    plugins_pool& pool = const_cast<plugins_registry&>(*this).assure<T>();
    return { pool.begin(), pool.end() };
  }

 private:
  std::vector<plugins_pool> pools_;
  std::unordered_map<std::string, std::pair<size_t, size_t>> names_map_;
};