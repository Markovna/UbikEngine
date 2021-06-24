#include "asset_handle.h"
#include "core/assets/texture.h"
#include "core/assets/shader.h"

#include <fstream>

namespace assets {


void init() {
  details::get_registry<texture>();
  details::get_registry<shader>();
}

}