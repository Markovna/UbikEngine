#pragma once

#include <string_view>
#include "gfx/experimental/gui.h"

class editor_tab {
 public:
  void begin(::gui* g) const;
  void end() const;

  [[nodiscard]] virtual std::string_view name() const = 0;
  virtual void gui() = 0;

  virtual ~editor_tab() = default;
};


