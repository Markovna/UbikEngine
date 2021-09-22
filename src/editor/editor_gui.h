#pragma once

#include "gui/gui.h"
#include "core/meta/type.h"
#include "base/type_name.h"

struct gui_renderer;

struct editor_gui {
  virtual void start(assets::provider*) = 0;
  virtual void gui(gui_renderer* gui_renderer) = 0;
  virtual ~editor_gui() = default;
};

struct editor_gui_plugin {
 public:
  void start(assets::provider* provider) {
    for (auto& editor : editors_) {
      if (editor)
        editor->start(provider);
    }
  }

  void gui(gui_renderer* gui_renderer) {
    for (auto& editor : editors_) {
      if (editor)
        editor->gui(gui_renderer);
    }
  }

  template<class T>
  T* get() {
    meta::typeid_t id = meta::get_typeid<T>();
    assert(editor_index_.count(id) > 0);

    return editors_[editor_index_[id]].get();
  }

  template<class T, class ...Args>
  void add_editor(Args... args) {

    meta::typeid_t id = meta::get_typeid<T>();
    assert(editor_index_.count(id) == 0);

    logger::core::Info("editor_gui_plugin::add_editor {}", meta::get_type<T>().name());

    editor_index_.insert(std::make_pair(id, editors_.size()));
    editors_.push_back(std::make_unique<T>(std::forward<Args>(args)...));
  }

  template<class T>
  void remove_editor() {
    meta::typeid_t id = meta::get_typeid<T>();
    assert(editor_index_.count(id) > 0);

    editors_[editor_index_[id]].reset();
    editor_index_.erase(id);
  }

 private:
  std::unordered_map<meta::typeid_t, size_t> editor_index_;
  std::vector<std::unique_ptr<editor_gui>> editors_;
};

namespace editor {
extern editor_gui_plugin *g_editor_gui;

void init_editor_gui();
void shutdown_editor_gui();
}