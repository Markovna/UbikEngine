#pragma once

struct world;
struct plugins_registry;
struct input_system;

class engine {
 public:
  engine() = default;
  void start();
  void update();
  void stop();

  input_system* input;
  plugins_registry* plugins;
  world* world;
};


