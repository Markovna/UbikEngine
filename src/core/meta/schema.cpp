#include "schema.h"
#include "core/assets/asset.h"
#include "base/log.h"
#include "platform/file_system.h"

#include <unordered_map>
#include <fstream>

namespace meta {

struct context {
  std::unordered_map<std::string, schema_info> schemas;
};

context* g_context;

void init() {
  g_context = new context;
}

void shutdown() {
  delete g_context;
}

void load_schemas(const char *path) {
  logger::core::Info("Load schemas from {}", path);

  fs::path schema_folder(path);

  for (const auto &entry : fs::directory_iterator(schema_folder)) {
    const fs::path &schema_path = entry.path();
    if (schema_path.extension().string() != ".schema")
      continue;

    logger::core::Info("Start parsing schema {}...", schema_path.c_str());

    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {

      file.open(schema_path, std::ios::in | std::ios::binary);
      asset data = asset::parse(file);

      schema_info &schema_info = g_context->schemas[data["name"]];
      schema_info.name = data["name"];
      schema_info.guid = guid::from_string(data["__guid"].get<std::string>().c_str());

      logger::core::Info("Schema {} added ({})", schema_info.name, schema_info.guid.str());

      file.close();
    }
    catch (std::ifstream::failure &e) {
      logger::core::Error("Couldn't read file {0} {1}", schema_path.string(), e.what());
    }
  }
}

guid get_schema_id(const char *name) {
  auto it = g_context->schemas.find(std::string(name));
  return it != g_context->schemas.end() ? it->second.guid : guid::invalid();
}

const schema_info &get_schema(const char *name) {
  return g_context->schemas[std::string(name)];
}

}