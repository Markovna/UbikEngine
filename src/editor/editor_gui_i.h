#pragma once

#include "gui/gui.h"
#include <imgui_internal.h>

struct engine;

struct gui_renderer;

struct editor_gui_i {
  virtual void gui(engine*, gui_renderer* gui_renderer) = 0;
  virtual ~editor_gui_i() = default;
};