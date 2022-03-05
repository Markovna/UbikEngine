#pragma once

#include "core/asset_repository.h"

struct version_component {
  asset_id id;
  uint32_t version;
  uint32_t components_version;
};