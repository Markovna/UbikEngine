#pragma once

struct plugins_registry;

extern "C" void load_spin_plugin(plugins_registry* reg);
extern "C" void unload_spin_plugin(plugins_registry *reg);
