#pragma once

namespace assets {
struct repository;
}

class application {
 public:
  virtual void start(assets::repository*) = 0;
  virtual void tick() = 0;
  virtual void stop() = 0;
  virtual ~application() = default;
};

extern application* g_application;

void set_application(application*);
void remove_application();
