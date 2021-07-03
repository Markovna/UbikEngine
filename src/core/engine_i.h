#pragma once

struct engine;

struct engine_i {
  virtual void start(engine*) = 0;
  virtual void stop(engine*) = 0;
  virtual void update(engine*) = 0;
  virtual ~engine_i() = default;
};