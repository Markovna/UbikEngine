#pragma once

#include "gui/gui.h"

struct gui_renderer;

struct editor_gui {
  virtual void gui(gui_renderer* gui_renderer) = 0;
  virtual ~editor_gui() = default;
};

struct editor_gui_plugin {
 public:
  void gui(gui_renderer* gui_renderer) {
    for (auto& editor : editors_) {
      editor->gui(gui_renderer);
    }
  }

  template<class T, class ...Args>
  void add_editor(Args... args) {
    editors_.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
  }

 private:
  std::vector<std::unique_ptr<editor_gui>> editors_;
};