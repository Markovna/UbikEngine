#pragma once

#include <string_view>
#include "gui.h"

class editor_tab {
 public:
  void begin(::gui* g) const;
  void end() const;

  bool is_hovered() const;
  [[nodiscard]] virtual std::string_view name() const = 0;
  virtual void gui() = 0;
  virtual void on_drop(const std::vector<std::string>&) {};

  virtual ~editor_tab() = default;

 protected:
};


