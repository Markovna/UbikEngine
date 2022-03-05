#pragma once

#include <sstream>

extern "C" void load_sandbox(std::istringstream*, struct systems_registry&);
extern "C" void unload_sandbox(std::ostringstream*, struct systems_registry&);


