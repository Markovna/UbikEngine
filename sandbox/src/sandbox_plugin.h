#pragma once

struct plugins_registry;

extern "C" void load_sandbox_plugin(plugins_registry* reg);
extern "C" void unload_sandbox_plugin(plugins_registry *reg);
