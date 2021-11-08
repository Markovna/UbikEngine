#pragma once

#include "gfx.h"

namespace experimental::gfx {

struct shader_blob_header_gl {
  uint32_t binding_table_offset;
  uint32_t binding_table_size;
  uint32_t source_offset;
  uint32_t source_size;
};

struct shader_binding_desc_gl {
  char name[64];
  uniform_type::type type;
  uint8_t binding;
};

}