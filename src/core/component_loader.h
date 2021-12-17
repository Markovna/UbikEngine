#pragma once

#include "assets/asset.h"

struct component_loader {
  void (*from_asset)(const asset&, struct world&, struct entity&);
  void (*instantiate)(struct world&, struct entity&);
};