#pragma once

#include <functional>

template<class T>
class interface;

template<class R, class ...A>
class interface<R(A...)> {
 public:
  using function_type = R(A...);

 public:
  template<class Func>
  explicit interface(Func func) : function_(std::move(func))
  {}

  template<class ...Args>
  R operator()(Args&&... args) const {
    return invoke(std::forward<Args>(args)...);
  }

  template<class ...Args>
  R invoke(Args&&... args) const {
    return function_(std::forward<Args>(args)...);
  }

 private:
  std::function<function_type> function_;
};