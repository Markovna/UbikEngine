#include "type_info.h"

#include <vector>
#include <unordered_map>

namespace meta::details {

registry& get_registry() {
  static registry inst;
  return inst;
}

void add_type(std::string_view name) {
  get_registry().create_or_get_type(name);
}

const type_info* get_type_info(std::string_view name) {
  return get_registry().get(name);
}

type_info* create_or_get_type_info(std::string_view name) {
  return get_registry().create_or_get_type(name);
}

}