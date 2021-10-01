#pragma once

#include <cstdint>

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
  Resize
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

struct window_event {
  event_type type = event_type::None;

  union {
    mouse_move_event mouse_move = {};
    mouse_up_event mouse_up;
    mouse_down_event mouse_down;
    scroll_event scroll;
    key_press_event key_press;
    key_release_event key_release;
    text_event text;
    close_event close;
    resize_event resize;
  };
};


