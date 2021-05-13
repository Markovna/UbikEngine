#include "input_system.h"

void input_system::push_event(window_event event) {
  switch (event.type) {
    case event_type::MouseUp: on_mouse_up(event.mouse_up); break;
    case event_type::MouseDown: on_mouse_down(event.mouse_down); break;
    case event_type::MouseMove: on_mouse_move(event.mouse_move); break;
    case event_type::KeyPress: on_key_press(event.key_press); break;
    case event_type::KeyRelease: on_key_release(event.key_release); break;
    case event_type::Scroll: on_scroll(event.scroll); break;
    case event_type::Text: on_text(event.text); break;
    case event_type::Close: on_close(event.close); break;
    case event_type::Resize: on_resize(event.resize); break;
    default: break;
  }
}
