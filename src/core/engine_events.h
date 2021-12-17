#pragma once

#include <functional>
#include "base/event.h"
#include "world.h"

class engine_events {
 public:
  using container_t = std::unordered_map<std::string, std::shared_ptr<void>>;
  using key_t = container_t::iterator;

  template<class ...Args>
  event<Args...>& get(const char* name) {
    assert(*name != 0);

    return *static_cast<event<Args...>*>(events_.find(name)->second.get());
  }

  template<class ...Args>
  event<Args...>& add(const char *name) {
    assert(*name != 0);

    auto [it, _] = events_.emplace(std::make_pair<std::string, std::shared_ptr<void>>(
        name,
        std::make_shared<event<Args...>>()
      ));
    return *static_cast<event<Args...>*>(it->second.get());
  }

  void remove(const char *name) {
    assert(*name != 0);

    auto it = events_.find(std::string(name));
    if (it == events_.end()) {}

    events_.erase(it);
  }

 private:
  container_t events_;
};
