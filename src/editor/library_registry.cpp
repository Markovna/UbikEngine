#include "library_registry.h"
#include "core/plugins_registry.h"
#include "base/log.h"
#include "platform/os.h"

#include <random>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

library_registry::library_registry(const char *libs_folder, const char *temp_folder)
    : libs_folder_(libs_folder)
    , temp_folder_(temp_folder)
    , file_watcher_()
{
  file_watcher_.add_path(libs_folder_.c_str(), false);
  file_watcher_.connect(this, &library_registry::on_file_changed);

  if (!std::filesystem::exists(temp_folder_))
    std::filesystem::create_directory(temp_folder_);
}

void *library_registry::load(const char *name, plugins_registry *registry) {
  assert(name_to_lib_info_.count(name) == 0 && "Library has been already loaded");

  fs::path src_lib_path = os::find_lib(libs_folder_.c_str(), name);
  fs::path tmp_lib_path = copy_to_temp(src_lib_path, temp_folder_);

  void* lib = os::load_lib(tmp_lib_path.c_str());
  if (!lib) {
    logger::core::Error("Couldn't load {} on path {} ({})", name, tmp_lib_path.c_str(), src_lib_path.c_str());
    fs::remove(tmp_lib_path);
  } else {
    filename_to_name_[src_lib_path.filename()] = name;
    lib_info &info = name_to_lib_info_[name];
    info.name = name;
    info.src_filename = src_lib_path.filename();
    info.tmp_filename = tmp_lib_path;
    info.symbols = lib;

    load_plugin(lib, name, registry);
  }
  return lib;
}

void library_registry::check_reload(plugins_registry *registry) {
  for (std::string& name : reloaded_) {
    lib_info& info = name_to_lib_info_[name];

    unload_plugin(info.symbols, info.name.c_str(), registry);

    // unload lib
    os::unload_lib(info.symbols);

    // remove temp lib file
    fs::remove(info.tmp_filename);

    // copy new lib to temp file
    fs::path src_lib_path = os::find_lib(libs_folder_.c_str(), name.c_str());
    fs::path tmp_lib_path = copy_to_temp(src_lib_path, temp_folder_);

    // load new version of plugin
    void* lib = os::load_lib(tmp_lib_path.c_str());
    if (!lib) {
      logger::core::Error("Couldn't load {} on path {}", name, tmp_lib_path.c_str());
      if (fs::exists(tmp_lib_path))
        fs::remove(tmp_lib_path);
    } else {
      info.tmp_filename = tmp_lib_path;
      info.symbols = lib;

      load_plugin(lib, name.c_str(), registry);
    }
  }
  reloaded_.clear();
}

void library_registry::unload(const char *name, plugins_registry *registry) {
  lib_info& info = name_to_lib_info_[name];

  // find and invoke unload function
  unload_plugin(info.symbols, name, registry);

  // unload lib
  os::unload_lib(info.symbols);

  // remove temp lib file
  fs::remove(info.tmp_filename);

  filename_to_name_.erase(info.src_filename);
  name_to_lib_info_.erase(info.name);
}

void library_registry::unload_all(plugins_registry* registry) {
  std::vector<std::string> names;
  for (auto& [name, _] : name_to_lib_info_) {
    names.push_back(name);
  }

  for (auto& name : names) {
    unload(name.c_str(), registry);
  }
}

static uint32_t get_random() {
  static std::random_device rd;
  static std::mt19937 mt(rd());
  static std::uniform_int_distribution<uint32_t> dist;
  return dist(mt);
}

std::filesystem::path library_registry::copy_to_temp(const fs::path &src, const fs::path &temp_folder) {
  fs::path tmp_lib_path(temp_folder);
  tmp_lib_path.append(std::to_string(get_random()));
  tmp_lib_path.replace_extension(src.extension());

  std::ifstream src_stream(src, std::ios::binary);
  std::ofstream dst_stream(tmp_lib_path, std::ios::binary);
  dst_stream << src_stream.rdbuf();
  return tmp_lib_path;
}

void library_registry::on_file_changed(const std::string &dir,
                                       const std::string &filename,
                                       file_action action,
                                       const std::string &old_name) {
  if (action != file_action::Modified) {
    if (filename_to_name_.count(filename)) {
      reloaded_.push_back(filename_to_name_[filename]);
    }
  }
}

void library_registry::load_plugin(void* lib, const char* name, plugins_registry* registry) {
  using load_func = void (*)(plugins_registry*);

  char load_function_name[50] = "load_";
  std::strcat(load_function_name, name);

  load_func load = (load_func) os::get_symbol(lib, load_function_name);
  if (!load) {
    logger::core::Error("Couldn't find load function for {}", name);
  } else {
    load(registry);
  }
}

void library_registry::unload_plugin(void* lib, const char* name, plugins_registry* registry) {
  using unload_func = void (*)(plugins_registry*);

  char unload_function_name[50] = "unload_";
  std::strcat(unload_function_name, name);

  unload_func load = (unload_func) os::get_symbol(lib, unload_function_name);
  if (!load) {
    logger::core::Error("Couldn't find unload function for {}", name);
  } else {
    load(registry);
  }
}
