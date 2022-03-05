#pragma once

#include "platform/file_system.h"
#include "platform/os.h"
#include "base/log.h"
#include "base/type_name.h"

#include <unordered_map>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>

class library_loader {
 private:
  template<class ...Args>
  using func_t = void(*)(Args...);

  struct lib_info {
    std::string name;
    fs::path src_path;
    fs::path temp_path;
    void* symbols;
    uint32_t version;
    int64_t timestamp;
  };

 public:
  explicit library_loader();
  ~library_loader();

  template<class ...Args>
  void load(const char* name, const fs::path& path, Args... args) {
    if (libs_.count(std::string(name))) {
      logger::core::Info("Library {} ({}) has been already loaded", name, path.c_str());
      return;
    }

    if (!fs::exists(path)) {
      logger::core::Error("Couldn't load library {} ({}), file doesn't exist", name, path.c_str());
      return;
    }

    fs::path temp_path = copy_to_temp(name, path, 0);

    void* symbols = os::load_lib(temp_path.c_str());
    if (!symbols) {
      logger::core::Error("Couldn't load {} from path {} ({})", name, temp_path.c_str(), path.c_str());
      fs::remove(temp_path);
      return;
    }

    std::istringstream* in = nullptr;
    invoke_load_function<std::istringstream*, Args...>(symbols, name, in, args...);

    libs_.insert({
           std::string(name),
           {
               .name = std::string(name),
               .src_path = path,
               .temp_path = temp_path,
               .symbols = symbols,
               .version = 0,
               .timestamp = os::get_timestamp(path),
           }
       });
  }

  template<class ...Args>
  void unload(const char* name, Args... args) {
    if (!libs_.count(std::string(name))) {
      logger::core::Info("Library {} has not been loaded", name);
      return;
    }

    lib_info& info = libs_[name];

    std::ostringstream* out = nullptr;
    invoke_unload_function(info.symbols, name, out, args...);

    os::unload_lib(info.symbols);

    fs::remove(info.temp_path);

    libs_.erase(info.name);
  }

  // unload all libs without invoking unload function and remove temporary paths
  void reset();

  template<class ...Args>
  void check_hot_reload(Args... args) {
    for (auto& [_, info] : libs_) {

      int64_t timestamp = os::get_timestamp(info.src_path);
      if (timestamp <= info.timestamp)
        continue;

      // reload
      fs::path temp_path = copy_to_temp(info.name.c_str(), info.src_path, info.version+1);
      void* symbols = os::load_lib(temp_path.c_str());
      if (!symbols) {
        // backup to previously loaded

        logger::core::Error("Couldn't reload {} from path {} ({})", info.name, temp_path.c_str(), info.src_path.c_str());
        fs::remove(temp_path);

        info.timestamp = os::get_timestamp(info.src_path);

      } else {

        std::ostringstream out { std::ios::binary | std::ios::out };

        // unload previously loaded
        invoke_unload_function<std::ostringstream*, Args...>(info.symbols, info.name.c_str(), &out, args...);

        std::istringstream in { out.str(), std::ios::binary | std::ios::in };
        invoke_load_function<std::istringstream*, Args...>(symbols, info.name.c_str(), &in, args...);

        os::unload_lib(info.symbols);

        // remove previously loaded lib file
        fs::remove(info.temp_path);

        info.temp_path = temp_path;

        info.symbols = symbols;
        info.timestamp = os::get_timestamp(info.src_path);
        info.version++;

        logger::core::Info("Reloaded library {} from {}", info.name, info.src_path.c_str());
      }
    }
  }

 private:
  fs::path copy_to_temp(const char* name, const fs::path& src_path, uint32_t version);

  template<class ...Args>
  void invoke_load_function(void* symbols, const char* name, Args... args) {
    char function_name[50] = "load_";
    std::strcat(function_name, name);
    invoke_function<Args...>(symbols, function_name, args...);
  }

  template<class ...Args>
  void invoke_unload_function(void* symbols, const char* name, Args... args) {
    char function_name[50] = "unload_";
    std::strcat(function_name, name);
    invoke_function<Args...>(symbols, function_name, args...);
  }

  template<class ...Args>
  void invoke_function(void* symbols, const char* function_name, Args... args) {
    auto func = (func_t<Args...>) os::get_symbol(symbols, function_name);

    if (!func) {
      logger::core::Error("Couldn't find function {}", function_name);
    } else {
      func(args...);
    }
  }

 private:
  fs::path temp_folder_;
  std::unordered_map<std::string, lib_info> libs_;
};