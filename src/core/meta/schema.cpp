#include "schema.h"
#include "base/log.h"
#include "core/serialization.h"

#include <unordered_map>
#include <filesystem>
#include <fstream>

namespace meta {

struct context {
  std::unordered_map<std::string, schema_info> schemas;
};

context& get_context() {
  static context inst;
  return inst;
}

void load_schemas(const char *path) {
  logger::core::Info("Load schemas from {}", path);

  std::filesystem::path schema_folder(path);

  for (const auto &entry : std::filesystem::directory_iterator(schema_folder)) {
    const std::filesystem::path &schema_path = entry.path();
    if (schema_path.extension().string() != ".schema")
      continue;

    logger::core::Info("Start parsing schema {}...", schema_path.c_str());

    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {

      file.open(schema_path, std::ios::in | std::ios::binary);
      asset data = nlohmann::json::parse(file);

      context &c = get_context();
      schema_info &schema_info = c.schemas[data["name"]];
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

const schema_info &get_schema(const char *name) {
  context &c = get_context();
  return c.schemas[std::string(name)];
}

}