#pragma once

#include "base/slot_map.h"

#include <unordered_map>
#include <string>

// TODO

template<class T>
std::unique_ptr<T>& get_shared() {
  static std::unique_ptr<T> ptr;
  return ptr;
}

class plugins {
 public:
  template<class T, class F = T, class ...Args>
  F* add(Args&&... args) {
    get_shared<T>().reset(new F(std::forward<Args>(args)...));
    return static_cast<F*>(get<T>());
  }

  template<class T>
  T* get() {
    return get_shared<T>().get();
  }

  template<class T>
  void remove() {
    get_shared<T>().reset();
  }
};

extern plugins* plugins_reg;

void init_plugins_registry();
void shutdown_plugins_registry();


