#pragma once

#include "base/window_event.h"
#include "base/cursor.h"
#include "base/timer.h"

#include "gfx/gfx.h"
#include "core/assets/asset_handle.h"

struct window;
struct ImGuiContext;
struct ImDrawData;
struct texture;

class gui_renderer {
 public:
  static std::unique_ptr<gui_renderer> create(window*);

  explicit gui_renderer(window*);
  ~gui_renderer();

  void begin_frame();
  void end_frame();

  void set_context();

  cursor::type cursor() const;

  void on_resize(resize_event&);
  void on_key_pressed(key_press_event& event);
  void on_key_released(key_release_event& event);
  void on_mouse_down(mouse_down_event& e);
  void on_mouse_up(mouse_up_event& e);
  void on_mouse_move(mouse_move_event& e);
  void on_scroll(scroll_event& e);
  void on_text_input(text_event& e);
 private:
  void render(ImDrawData* draw_data);

 private:
  constexpr static const size_t kBufferMaxSize  = 10 * 2048;

  shader_handle shader_;
  std::unique_ptr<texture> texture_;
  gfx::uniform_handle texture_uniform_handle_;
  gfx::vertexbuf_handle vb_handle_;
  gfx::indexbuf_handle ib_handle_;
  ImGuiContext* context_;
  mutable cursor::type cursor_ = cursor::Arrow;
  timer timer_;
};



