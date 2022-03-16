#pragma once

#include "base/cursor.h"
#include "core/texture.h"
#include "base/timer.h"
#include "gfx/gfx.h"
#include "core/systems_registry.h"

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
struct drop_event;
struct renderer;
struct shader_repository;

void gui_begin_dockspace();

class gui {
 public:
  explicit gui(systems_registry&);

  void init(window* w);
  void begin();
  void end(framebuf_handle);

  void on_resize(const resize_event&);
  void on_key_pressed(const key_press_event&);
  void on_key_released(const key_release_event&);
  void on_mouse_down(const mouse_down_event&);
  void on_mouse_up(const mouse_up_event&);
  void on_mouse_move(const mouse_move_event&);
  void on_scroll(const scroll_event&);
  void on_text_input(const text_event&);

  cursor::type cursor() const;
  void set_context() const;

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
  system_ptr<renderer> renderer_;
  system_ptr<shader_repository> shader_repository_;
  mutable cursor::type cursor_ = cursor::Arrow;
};

void connect_gui_events(gui&, class input_system&);
void disconnect_gui_events(gui&, class input_system&);
