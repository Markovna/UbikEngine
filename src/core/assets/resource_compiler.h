#pragma once

#include "core/assets/filesystem_provider.h"

#include <fstream>

namespace resources {

class compiler {
 public:
  using compile_func_t = bool(*)(std::ifstream& stream, const asset& meta, std::ostream&);

  void register_compiler(const std::string& type_name, compile_func_t compile_func) {
    compilers_.emplace(std::make_pair(type_name, compile_func));
  }

  void unregister_compiler(const std::string& type_name) {
    compilers_.erase(type_name);
  }

  bool compile(const fs::path& path) {
    assets::handle meta = assets::load(g_fsprovider, path);

    auto it = compilers_.find(meta->at("type"));
    if (it == compilers_.end())
      return false;

    // remove old buffer if exists
    if (meta->contains("data")) {
      filesystem_provider::remove_buffer(path, meta->at("data"));
    }

    compile_func_t compile_func = it->second;
    std::stringstream out;
    std::ifstream file = fs::read_file(fs::absolute(meta->at("source_path")));

    bool success = compile_func(file, *meta, out);
    if (success) {
      uint64_t buffer_id = assets::add_buffer(meta, "data");
      filesystem_provider::save_buffer(path, buffer_id, out);
      g_fsprovider->save(path, *meta);
      return true;
    }

    return false;
  }

 private:
  std::unordered_map<std::string, compile_func_t> compilers_;
};

extern compiler *g_compiler;
void init_compiler();
void shutdown_compiler();
void compile_all_assets(const char *directory);

}