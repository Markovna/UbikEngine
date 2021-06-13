#include "type_info.h"

#include <vector>
#include <unordered_map>

namespace meta::details {

class registry {
 public:
  void add_type(type_info&& info) {
    name_index_[info.name] = types_.size();
    id_index_[info.id] = types_.size();
    types_.emplace_back(std::move(info));
  }

  [[nodiscard]] const type_info& get(guid id) const {
    if (auto it = id_index_.find(id); it != id_index_.end()) {
      return types_[it->second];
    }

    return type_info::invalid();
  }

  [[nodiscard]] const type_info& get(const char* name) const {
    if (auto it = name_index_.find(name); it != name_index_.end()) {
      return types_[it->second];
    }

    return type_info::invalid();
  }

 private:
  std::vector<type_info> types_;
  std::unordered_map<std::string, uint32_t> name_index_;
  std::unordered_map<guid, uint32_t> id_index_;
};

registry& get_registry() {
  static registry inst;
  return inst;
}

void add_type(type_info&& info) {
  get_registry().add_type(std::move(info));
}

const type_info& get_type(guid id) {
  return get_registry().get(id);
}

const type_info& get_type(const char* name) {
  return get_registry().get(name);
}

const type_info& type_info::invalid() {
  static type_info inst {.id = guid::invalid(), .name = ""};
  return inst;
}

}