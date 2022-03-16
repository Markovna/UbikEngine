#pragma once

#include <sstream>

extern "C" void load_plugin(std::istringstream*, struct systems_registry&);
extern "C" void unload_plugin(std::ostringstream*, struct systems_registry&);


