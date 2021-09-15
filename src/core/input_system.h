#pragma once

#include "base/window_event.h"
#include "base/event.h"

class input_system {
 public:
  event<mouse_move_event&> on_mouse_move;
  event<mouse_up_event&> on_mouse_up;
  event<mouse_down_event&> on_mouse_down;
  event<key_press_event&> on_key_press;
  event<key_release_event&> on_key_release;
  event<scroll_event&> on_scroll;
  event<text_event&> on_text;
  event<close_event&> on_close;
  event<resize_event&> on_resize;

 public:
  void push_event(window_event);
};

extern input_system* input;

void init_input_system();
void shutdown_input_system();


