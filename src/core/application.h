#pragma once

class application {
 public:
  virtual void start() = 0;
  virtual void tick() = 0;
  virtual void stop() = 0;
  virtual ~application() = default;
};

