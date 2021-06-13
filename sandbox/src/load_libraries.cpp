#include "core/plugins_registry.h"
#include "spin_plugin.h"
#include "sandbox_plugin.h"

void load_plugins(engine* engine) {
  load_spin_plugin(engine);
  load_sandbox_plugin(engine);
}