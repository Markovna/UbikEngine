#include "vertex_layout_desc.h"

vertex_layout_desc& vertex_layout_desc::add(
    vertex_semantic::type semantic,
    vertex_type::type type,
    uint32_t size,
    bool normalized) {

  items[semantic].size = size;
  items[semantic].type = type;
  items[semantic].offset = offset;
  items[semantic].normalized = normalized;

  offset += size * vertex_type::sizes[type];

  return *this;
}

vertex_layout_desc::operator vertex_layout() const {
  vertex_layout layout { };
  std::memcpy(layout.items, items, vertex_semantic::COUNT * sizeof(vertex_layout::item));
  layout.stride = offset;
  return layout;
}
