#pragma once

#include "core/meta/interface.h"

class load_component_interface : public interface<void(const class asset&, struct world&, struct entity&)> {
  using interface::interface;
};

class instantiate_component_interface : public interface<void(struct world&, struct entity&)> {
  using interface::interface;
};