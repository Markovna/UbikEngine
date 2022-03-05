#pragma once
#include "assets_filesystem.h"
#include "meta/type_info.h"
#include "base/slot_map.h"

class schema_registry;

struct schema_handle {
  uint32_t idx = std::numeric_limits<uint32_t>::max();
  uint32_t gen = std::numeric_limits<uint32_t>::max();
};

enum class schema_type : uint8_t {
  FLOAT  = 0,
  INT64,
  BOOL,
  STRING,
  OBJECT,
  ARRAY,

  COUNT
};

struct schema_property {
  std::string name;
  schema_type type;
  schema_handle schema; // for objects & arrays of objects
};

struct schema {
  std::string name;
  std::vector<schema_property> properties;
};

class schema_builder {
 public:
  explicit schema_builder(meta::typeid_t id) : type_id(id), properties_() {}
  const schema& build(schema_registry&);
  schema_builder& add(std::string_view, schema_type, meta::typeid_t = {});

 private:
  struct property {
    std::string name;
    schema_type type;
    meta::typeid_t type_id;
  };
  std::vector<property> properties_;
  meta::typeid_t type_id;
};

void build_schema_from_asset(meta::typeid_t, asset& asset, schema_registry& registry);

class schema_registry {
 public:
  using container = stdext::slot_map<schema, schema_handle>;
  using const_iterator = container::const_iterator;

  schema_handle register_schema(meta::typeid_t, schema);
  const_iterator find_schema(meta::typeid_t) const;
  const schema& get_schema(schema_handle) const;
  schema_handle get_schema_handle(meta::typeid_t);
  bool is_valid(schema_handle);

  const_iterator begin() const;
  const_iterator end() const;

 private:
  container schemas_;
  std::unordered_map<meta::typeid_t, schema_handle> type_index_;
};
