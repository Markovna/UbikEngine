#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace meta {

using typeid_t = uintptr_t;

namespace details {

struct type_info {
  typeid_t id;
  std::string name;
};

class registry {
 public:
  [[nodiscard]] const type_info* get(std::string_view name) const {
    if (auto it = types_name_index_.find(std::string { name }); it != types_name_index_.end()) {
      return (type_info*) it->second;
    }

    return nullptr;
  }

  type_info* create_or_get_type(std::string_view name);

 private:
  std::vector<std::unique_ptr<type_info>> types_;
  std::unordered_map<std::string, typeid_t> types_name_index_;
};

registry& get_registry();

inline const type_info* get_type_info(typeid_t id) {
  return (type_info*) id;
}

inline bool has_type_info(typeid_t id) {
  return (type_info*) id;
}

type_info* add_type(std::string_view name);
const type_info* get_type_info(std::string_view);
type_info* create_or_get_type_info(std::string_view);

}

}