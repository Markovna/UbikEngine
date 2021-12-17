#pragma once

#include "base/event.h"

class world;

class simulation_events {
 public:
  event<world*> start;
  event<> update;
  event<> stop;
};