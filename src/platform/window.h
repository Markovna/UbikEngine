#pragma once

#include <queue>

#include "base/math.h"
#include "base/cursor.h"
#include "base/window_event.h"

class window {
 public:
  using window_handle = void*;

 public:
  explicit window(vec2i size);
  ~window();

  void update();
  bool poll_event(window_event&);
  void set_cursor(cursor::type);

  [[nodiscard]] window_handle get_handle() const;
  [[nodiscard]] vec2i get_size() const;
  [[nodiscard]] vec2i get_resolution() const;

 private:
  template<class ...Args>
  void emplace_event(Args&&... args) {
    events_.emplace(std::forward<Args>(args)...);
  }

 private:
  vec2i size_;
  vec2i resolution_;
  window_handle handle_;
  std::queue<window_event> events_;
};