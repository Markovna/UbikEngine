#pragma once

#include "platform/file_watcher.h"

#include <filesystem>
#include <unordered_map>
#include <vector>

struct plugins_registry;
struct engine;

class library_registry {
 private:

 public:
  library_registry(const char* libs_folder, const char* temp_folder);
  ~library_registry() = default;

  void* load(const char* name, engine*);
  void unload(const char* name, engine* registry);
  void unload_all(engine* registry);
  void check_reload(engine* registry);

 private:
  void load_plugin(void*, const char*, engine*);
  void unload_plugin(void*, const char*, engine*);

  static std::filesystem::path copy_to_temp(const std::filesystem::path& src, const std::filesystem::path& temp_folder);
  void on_file_changed(const std::string& dir, const std::string& filename, file_action action, const std::string& old_name);

 private:
  struct lib_info {
    std::string name;
    std::string src_filename;
    std::string tmp_filename;
    void* symbols;
  };

  std::unordered_map<std::string, lib_info> name_to_lib_info_;
  std::unordered_map<std::string, std::string> filename_to_name_;

  std::filesystem::path libs_folder_;
  std::filesystem::path temp_folder_;
  file_watcher file_watcher_;

  // TODO: mutex
  std::vector<std::string> reloaded_;
};


