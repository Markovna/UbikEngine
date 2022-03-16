#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "base/math.h"
#include "base/mouse_codes.h"
#include "base/key_codes.h"

enum class event_type : uint8_t {
  None,
  MouseUp,
  MouseDown,
  MouseMove,
  Scroll,
  KeyPress,
  KeyRelease,
  Text,
  Close,
  Resize,
  Drop
};

struct mouse_move_event {
  float x;
  float y;
};

struct mouse_up_event {
  mouse_code mouse_code;
};

struct mouse_down_event {
  mouse_code mouse_code;
};

struct scroll_event {
  float x;
  float y;
};

struct key_press_event {
  key_code key_code;
  bool control;
  bool shift;
  bool alt;
  bool super;
  bool repeat;
};

struct key_release_event {
  key_code key_code;
  bool control;
  bool shift;
  bool alt;
  bool super;
};

struct text_event {
  uint32_t unicode;
};

struct close_event {
};

struct resize_event {
  vec2i resolution;
  vec2i size;
};

struct drop_event {
  std::vector<std::string> paths {};
};

class window_event {
 public:
  window_event()                        : type_(event_type::None),       value_() {}
  window_event(mouse_move_event&& val)  : type_(event_type::MouseMove),  value_(std::move(val)) {}
  window_event(mouse_up_event&& val)    : type_(event_type::MouseUp),    value_(std::move(val)) {}
  window_event(mouse_down_event&& val)  : type_(event_type::MouseDown),  value_(std::move(val)) {}
  window_event(scroll_event&& val)      : type_(event_type::Scroll),     value_(std::move(val)) {}
  window_event(key_press_event&& val)   : type_(event_type::KeyPress),   value_(std::move(val)) {}
  window_event(key_release_event&& val) : type_(event_type::KeyRelease), value_(std::move(val)) {}
  window_event(text_event&& val)        : type_(event_type::Text),       value_(std::move(val)) {}
  window_event(close_event&& val)       : type_(event_type::Close),      value_(std::move(val)) {}
  window_event(resize_event&& val)      : type_(event_type::Resize),     value_(std::move(val)) {}
  window_event(drop_event&& val)        : type_(event_type::Drop),       value_(std::move(val)) {}

  operator const mouse_move_event&()  const { assert(type_ == event_type::MouseMove);  return value_.mouse_move; }
  operator const mouse_up_event&()    const { assert(type_ == event_type::MouseUp);    return value_.mouse_up; }
  operator const mouse_down_event&()  const { assert(type_ == event_type::MouseDown);  return value_.mouse_down; }
  operator const scroll_event&()      const { assert(type_ == event_type::Scroll);     return value_.scroll; }
  operator const key_press_event&()   const { assert(type_ == event_type::KeyPress);   return value_.key_press; }
  operator const key_release_event&() const { assert(type_ == event_type::KeyRelease); return value_.key_release; }
  operator const text_event&()        const { assert(type_ == event_type::Text);       return value_.text; }
  operator const close_event&()       const { assert(type_ == event_type::Close);      return value_.close; }
  operator const resize_event&()      const { assert(type_ == event_type::Resize);     return value_.resize; }
  operator const drop_event&()        const { assert(type_ == event_type::Drop);       return *value_.drop; }

  window_event(const window_event& other) = delete;
  window_event& operator=(const window_event& other) = delete;

  window_event(window_event&& other) : type_(std::move(other.type_)), value_(std::move(other.value_)) {
    other.type_ = event_type::None;
    other.value_ = { };
  }

  window_event& operator=(window_event&& other) {
    window_event tmp(std::move(other));
    using std::swap;
    swap(type_, tmp.type_);
    swap(value_, tmp.value_);
    return *this;
  }

  event_type type() const { return type_; }

  ~window_event() { value_.destroy(type_); }

 private:
  union value {
    std::nullptr_t none = {};
    mouse_move_event mouse_move;
    mouse_up_event mouse_up;
    mouse_down_event mouse_down;
    scroll_event scroll;
    key_press_event key_press;
    key_release_event key_release;
    text_event text;
    close_event close;
    resize_event resize;
    drop_event* drop;

    value() : none() { }
    value(mouse_move_event&& val) : mouse_move(val) { }
    value(mouse_up_event&& val) : mouse_up(val) { }
    value(mouse_down_event&& val) : mouse_down(val) { }
    value(scroll_event&& val) : scroll(val) { }
    value(key_press_event&& val) : key_press(val) { }
    value(key_release_event&& val) : key_release(val) { }
    value(text_event&& val) : text(val) { }
    value(close_event&& val) : close(val) { }
    value(resize_event&& val) : resize(val) { }
    value(drop_event&& val) : drop(new drop_event(val)) { }

    void destroy(event_type t) {
      if (t == event_type::Drop) {
        assert(drop != nullptr);
        delete drop;
      }
    }
  };

  event_type type_;
  value value_;
};


