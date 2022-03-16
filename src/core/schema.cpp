#include "schema.h"
#include "meta/type.h"
#include "asset_repository.h"

#include <array>

constexpr static std::array<const char*, (size_t) schema_type::COUNT> schema_type_names {
    "float",
    "int",
    "bool",
    "string",
    "object",
    "array"
};

schema_type get_schema_type(std::string_view name) {
  auto it = std::find(schema_type_names.begin(), schema_type_names.end(), name);
  return (schema_type) std::distance(schema_type_names.begin(), it);
}

schema_registry::const_iterator schema_registry::find_schema(meta::typeid_t id) const {
  auto it = type_index_.find(id);
  if (it != type_index_.end()) {
    return schemas_.find(it->second);
  }
  return schemas_.end();
}

const schema& schema_registry::get_schema(schema_handle handle) const {
  return schemas_.at(handle);
}

schema_handle schema_registry::register_schema(meta::typeid_t id, schema schema) {
  return type_index_[id] = schemas_.emplace(std::move(schema));
}

schema_registry::const_iterator schema_registry::begin() const {
  return schemas_.begin();
}

schema_registry::const_iterator schema_registry::end() const {
  return schemas_.end();
}

schema_handle schema_registry::get_schema_handle(meta::typeid_t id) {
  auto it = type_index_.find(id);
  if (it != type_index_.end()) {
    return it->second;
  }
  return { };
}

bool schema_registry::is_valid(schema_handle handle) {
  return schemas_.find(handle) != schemas_.end();
}

void build_schema_from_asset(meta::typeid_t id, asset& a, schema_registry& registry) {
  schema schema;
  schema.name = a.at("name").get<std::string>();
  const asset_array& props = a.at("properties");

  for (const asset& prop : props) {
    auto prop_type = get_schema_type(prop.at("type"));
    if (prop_type == schema_type::OBJECT) {
      auto obj_schema = registry.get_schema_handle(meta::get_type(prop.at("schema").get<std::string>()).id());
      if (registry.is_valid(obj_schema)) {
        schema.properties.push_back(schema_property {
            .name = prop.at("name"),
            .type = prop_type,
            .schema = obj_schema
        });
      }
    } else {
      schema.properties.push_back(schema_property {
          .name = prop.at("name"),
          .type = prop_type,
      });
    }
  }

  registry.register_schema(id, std::move(schema));
}

schema_builder& schema_builder::add(std::string_view name, schema_type type, meta::typeid_t type_id) {
  properties_.push_back(property {
    .name = std::string(name),
    .type = type,
    .type_id = type_id
  });
  return *this;
}

const schema &schema_builder::build(schema_registry& reg) {
  std::vector<schema_property> properties;
  for (auto& prop : properties_) {
    properties.push_back(schema_property
        {
          .name = prop.name,
          .type = prop.type,
          .schema = reg.get_schema_handle(prop.type_id)
        });
  }

  auto handle = reg.register_schema(type_id, schema {
    .name = std::string(meta::type(type_id).name()),
    .properties = std::move(properties)
  });
  return reg.get_schema(handle);
}
