#pragma once

class world;

class simulation {
 public:
  virtual void start(world&) = 0;
  virtual void update(world&, float dt) = 0;
  virtual void stop() = 0;
  virtual ~simulation() = default;
};

