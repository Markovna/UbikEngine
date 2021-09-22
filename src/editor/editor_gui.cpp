#include "editor_gui.h"
namespace editor {
editor_gui_plugin *g_editor_gui;

void init_editor_gui() {
  g_editor_gui = new editor_gui_plugin;
}
void shutdown_editor_gui() {
  delete g_editor_gui;
}
}