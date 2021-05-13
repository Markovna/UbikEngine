#include "core/plugins_registry.h"
#include "spin_plugin.h"

void load_plugins(plugins_registry* reg) {
  load_spin_plugin(reg);
}