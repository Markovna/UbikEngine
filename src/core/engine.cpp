#include "engine.h"
#include "plugins_registry.h"

void engine::update() {
  for (auto& plugin : *plugins) {
      plugin->update(this);
  }
}

void engine::start() {
  for (auto& plugin : *plugins) {
      plugin->start(this);
  }
}

void engine::stop() {
  for (auto& plugin : *plugins) {
      plugin->stop(this);
  }
}
