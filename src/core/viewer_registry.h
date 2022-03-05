#pragma once

#include <vector>
#include "viewer.h"

class viewer_registry {
 public:
  [[nodiscard]] auto begin() const { return viewers_.begin(); }
  [[nodiscard]] auto end() const { return viewers_.end(); }

  void request_render(viewer viewer);
  void clear();

 private:
  std::vector<viewer> viewers_;
};


