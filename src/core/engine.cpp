#include "engine.h"
#include "plugins_registry.h"
#include "engine_i.h"

void engine::update() {
  for (engine_i& plugin : plugins->view<engine_i>()) {
    plugin.update(this);
  }
}

void engine::start() {
  for (engine_i& plugin : plugins->view<engine_i>()) {
    plugin.start(this);
  }
}

void engine::stop() {
  for (engine_i& plugin : plugins->view<engine_i>()) {
    plugin.stop(this);
  }
}
