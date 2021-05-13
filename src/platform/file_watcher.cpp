#include "file_watcher.h"

#include <efsw/efsw.hpp>

class update_listener : public efsw::FileWatchListener {
 public:
  using event_t = event<const std::string&, const std::string&, file_action, const std::string&>;

 public:
  explicit update_listener(event_t& event) : event_(event) {}

  void handleFileAction( efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename) override
  {
    switch (action)
    {
      case efsw::Actions::Add:
        event_(dir, filename, file_action::Add, "");
        break;
      case efsw::Actions::Delete:
        event_(dir, filename, file_action::Delete, "");
        break;
      case efsw::Actions::Modified:
        event_(dir, filename, file_action::Modified, "");
        break;
      case efsw::Actions::Moved:
        event_(dir, filename, file_action::Moved, oldFilename);
        break;
      default:
        std::cerr << "Should never happen!" << std::endl;
    }
  }
 private:
  event_t& event_;
};

file_watcher::file_watcher()
    : on_modified_()
    , watcher_(new efsw::FileWatcher)
    , listener_(new update_listener(on_modified_))
{
  watcher_->watch();
}

void file_watcher::add_path(const char *path, bool recursive) {
  watcher_->addWatch(path, listener_, recursive);
}
void file_watcher::remove_path(const char *path) {
  watcher_->removeWatch(path);
}
void file_watcher::disconnect(file_watcher::handler_t &&handler) {
  on_modified_.disconnect(std::forward<handler_t>(handler));
}
void file_watcher::disconnect_all() {
  on_modified_.disconnect_all();
}
file_watcher::~file_watcher() {
  disconnect_all();
  delete watcher_;
  delete listener_;
}
