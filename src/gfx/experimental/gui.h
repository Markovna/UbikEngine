#pragma once

#include "base/cursor.h"
#include "texture.h"
#include "base/timer.h"
#include "gfx.h"

struct renderer;
struct shader;
struct window;
struct ImGuiContext;
struct input_system;

struct resize_event;
struct key_press_event;
struct key_release_event;
struct mouse_down_event;
struct mouse_up_event;
struct mouse_move_event;
struct scroll_event;
struct text_event;

void gui_begin_dockspace();

class gui {
 public:
  explicit gui(window*);
  void begin();
  void end(framebuf_handle);

  void on_resize(resize_event&);
  void on_key_pressed(key_press_event&);
  void on_key_released(key_release_event&);
  void on_mouse_down(mouse_down_event&);
  void on_mouse_up(mouse_up_event&);
  void on_mouse_move(mouse_move_event&);
  void on_scroll(scroll_event&);
  void on_text_input(text_event&);

  cursor::type cursor() const;

 private:
  void render(framebuf_handle, struct ImDrawData*);

 private:
  timer timer_;
  vertexbuf_handle vb_handle_;
  indexbuf_handle ib_handle_;
  uniform_handle camera_uniform_handle_;
  uniform_handle texture_uniform_handle_;
  uniformbuf_handle camera_buffer_;
  std::unique_ptr<texture> font_texture_;
  bool size_changed_;
  shader* shader_;
  ImGuiContext* context_;
  mutable cursor::type cursor_ = cursor::Arrow;
};

void load_gui(struct systems_registry&);

void connect_gui_events(gui&, class input_system&);
void disconnect_gui_events(gui&, class input_system&);
