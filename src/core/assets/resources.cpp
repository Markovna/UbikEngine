#include <core/assets/texture.h>
#include <core/assets/shader.h>
#include "resources.h"


namespace resources {

void init() {
  register_pool<texture>(::assets::loader::load<texture>);
  register_pool<shader>(::assets::loader::load<shader>);
}

void shutdown() {
  unregister_pool<texture>();
  unregister_pool<shader>();
}

}