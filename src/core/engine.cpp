#include "engine.h"
#include "plugins_registry.h"

void engine::update() {
  for (plugin& plugin : *plugins) {
      plugin.update(this);
  }
}

void engine::start() {
  for (plugin& plugin : *plugins) {
      plugin.start(this);
  }
}

void engine::stop() {
  for (plugin& plugin : *plugins) {
      plugin.stop(this);
  }
}
