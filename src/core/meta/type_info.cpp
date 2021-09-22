#include "type_info.h"

#include <vector>
#include <unordered_map>

namespace meta::details {

class registry {
 public:
  type_info* add_type(type_info&& info) {
    type_info* ptr;
    if (auto it = name_index_.find(info.name); it != name_index_.end()) {
      ptr = (type_info*) it->second;
      *ptr = std::move(info);
    } else {
     ptr = types_.emplace_back(std::make_unique<type_info>(std::move(info))).get();
    }
    ptr->id = (uintptr_t) ptr;
    name_index_[ptr->name] = ptr->id;
    return ptr;
  }

//  [[nodiscard]] const type_info* get(const char* name) const {
//    if (auto it = name_index_.find(name); it != name_index_.end()) {
//      return (type_info*) it->second;
//    }
//
//    return nullptr;
//  }

  [[nodiscard]] const type_info* get(std::string_view name) const {
    if (auto it = name_index_.find(std::string { name }); it != name_index_.end()) {
      return (type_info*) it->second;
    }

    return nullptr;
  }

 private:
  std::vector<std::unique_ptr<type_info>> types_;
  std::unordered_map<std::string, typeid_t> name_index_;
};

registry& get_registry() {
  static registry inst;
  return inst;
}

void add_type(type_info&& info) {
  get_registry().add_type(std::move(info));
}

const type_info* get_type_info(std::string_view name) {
  return get_registry().get(name);
}

}