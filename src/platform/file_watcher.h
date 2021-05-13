#pragma once

#include "base/event.h"
#include "base/delegate.h"

#include <iostream>

struct update_listener;

namespace efsw {

struct FileWatcher;

}

enum class file_action {
  Add = 0,
  Delete,
  Modified,
  Moved
};

class file_watcher {
 public:
  using handler_t = delegate<void(const std::string&, const std::string&, file_action, const std::string&)>;
  using event_t = event<const std::string&, const std::string&, file_action, const std::string&>;

  file_watcher();
  ~file_watcher();

  void add_path(const char* path, bool recursive);
  void remove_path(const char* path);

  template<class ...Args>
  void connect(Args&&... handler) {
    on_modified_.connect(std::forward<Args>(handler)...);
  }

  void disconnect(handler_t&& handler);
  void disconnect_all();

 private:
  event_t on_modified_;
  efsw::FileWatcher* watcher_;
  update_listener* listener_;

};


