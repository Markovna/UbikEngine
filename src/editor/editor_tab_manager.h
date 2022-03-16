#pragma once

#include "core/systems_registry.h"

class editor_tab;
class gui;
class input_system;
class drop_event;

class editor_tab_manager {
 public:
  explicit editor_tab_manager(systems_registry&);
  ~editor_tab_manager();

  void update();

 private:
  void on_drop(const drop_event&);

 private:
  systems_registry::pool_view<editor_tab> editor_tab_view_;
  system_ptr<gui> gui_renderer_;
  system_ptr<input_system> input_system_;
  std::vector<std::string> drop_paths_;
};


