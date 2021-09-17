#pragma once

namespace assets {
struct provider;
}

class application {
 public:
  virtual void start(assets::provider*) = 0;
  virtual void tick() = 0;
  virtual void stop() = 0;
  virtual ~application() = default;
};

extern application* g_application;

void set_application(application*);
void remove_application();
