#include "plugins_registry.h"

void plugins_registry::remove_plugin(const char *name) {
  if (names_map_.count(std::string(name))) {
    logger::core::Info("plugins_registry::remove_plugin {}", name);

    std::string key(name);
    plugins_.erase(names_map_[key]);
    names_map_.erase(key);
  }
}