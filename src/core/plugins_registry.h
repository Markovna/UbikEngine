#pragma once

#include "base/log.h"
#include "base/slot_map.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

struct engine;

class plugin_base {
 public:
  virtual void update(engine* e) {};
  virtual void start(engine* e) {};
  virtual void stop(engine* e) {};

  virtual ~plugin_base() = default;
};

class plugins_registry {
 public:
  using container = stdext::slot_map<std::unique_ptr<plugin_base>>;
  using iterator = container::iterator;
  using key = container::key_type;

 public:
  template<class T, class ...Args>
  T* add_plugin(const char* name, Args... args) {
    assert(names_map_.count(std::string(name)) == 0 && "Plugin has been already added");

    logger::core::Info("plugins_registry::add_plugin {}", name);

    key key = names_map_[std::string(name)] = plugins_.emplace(std::make_unique<T>(std::forward<Args>(args)...));
    return static_cast<T*>(plugins_[key].get());
  }

  template<class T>
  T* get_plugin(const char* name) {
    if (auto it = names_map_.find(std::string(name)); it != names_map_.end()) {
      return static_cast<T*>(plugins_[it->second].get());
    }
    return nullptr;
  }

  void remove_plugin(const char* name);

  iterator begin() { return plugins_.begin(); }
  iterator end() { return plugins_.end(); }

 private:
  container plugins_;
  std::unordered_map<std::string, key> names_map_;
};

