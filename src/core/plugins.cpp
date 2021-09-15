#include "plugins.h"

plugins* plugins_reg;

void init_plugins_registry() {
  plugins_reg = new plugins;
}
void shutdown_plugins_registry() {
  delete plugins_reg;
}