#pragma once

#include "gfx.h"

namespace experimental::gfx {

class vertex_layout_desc {
 public:
  vertex_layout_desc& add(vertex_semantic::type, vertex_type::type, uint32_t size, bool normalized = false);
  vertex_layout_desc& skip(uint32_t offs) { offset += offs; return *this; }

  explicit operator vertex_layout() const;

 private:
  vertex_layout::item items[vertex_semantic::COUNT];
  uint32_t offset;
};

}