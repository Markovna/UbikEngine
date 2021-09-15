#include "core/plugins_registry.h"
#include "spin_plugin.h"
#include "sandbox_plugin.h"

void load_plugins(plugins* plugins_registry) {
  load_spin_plugin(plugins_registry);
  load_sandbox_plugin(plugins_registry);
}