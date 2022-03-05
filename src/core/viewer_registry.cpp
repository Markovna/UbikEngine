#include "viewer_registry.h"

void viewer_registry::request_render(viewer viewer) {
  viewers_.emplace_back(std::move(viewer));
}

void viewer_registry::clear() {
  viewers_.clear();
}
