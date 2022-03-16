#pragma once

#include "base/window_event.h"
#include "base/event.h"

class input_system {
 public:
  event<const mouse_move_event&> on_mouse_move;
  event<const mouse_up_event&> on_mouse_up;
  event<const mouse_down_event&> on_mouse_down;
  event<const key_press_event&> on_key_press;
  event<const key_release_event&> on_key_release;
  event<const scroll_event&> on_scroll;
  event<const text_event&> on_text;
  event<const close_event&> on_close;
  event<const resize_event&> on_resize;
  event<const drop_event&> on_drop;

 public:
  void push_event(const window_event&);
};


