#include "spin_plugin.h"
#include "sandbox_plugin.h"

void load_plugins(struct plugins* plugins_registry) {
  load_spin_plugin(plugins_registry);
  load_sandbox_plugin(plugins_registry);
}