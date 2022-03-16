#include "input_system.h"

void input_system::push_event(const window_event& event) {
  switch (event.type()) {
    case event_type::MouseUp: on_mouse_up(event); break;
    case event_type::MouseDown: on_mouse_down(event); break;
    case event_type::MouseMove: on_mouse_move(event); break;
    case event_type::KeyPress: on_key_press(event); break;
    case event_type::KeyRelease: on_key_release(event); break;
    case event_type::Scroll: on_scroll(event); break;
    case event_type::Text: on_text(event); break;
    case event_type::Close: on_close(event); break;
    case event_type::Resize: on_resize(event); break;
    case event_type::Drop: on_drop(event); break;
    default: break;
  }
}
