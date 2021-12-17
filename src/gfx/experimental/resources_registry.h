#pragma once

#include <memory>
#include <unordered_map>
#include <core/assets/asset.h>
#include "base/guid.h"
#include "base/delegate.h"
#include "platform/file_system.h"
#include "base/slot_map.h"
#include "gfx/experimental/asset_repository.h"

template<class T, class ...Args>
class resources_registry {
 public:
  using handle = std::unique_ptr<T>;

  handle load(const fs::path& path, Args... args) {
    loader_(*assets_->load(path), assets_, std::forward<Args>(args)...);
  }

  handle load(const guid& id) {

  }

 private:
  using key_t = stdext::slot_map<struct _>::key_type;

  assets_repository* assets_;
  delegate<T(const asset&, assets_repository*, Args...)> loader_;

  stdext::slot_map<std::weak_ptr<T>> resources_;
  std::unordered_map<fs::path, key_t> path_index_;
  std::unordered_map<guid, key_t> id_index_;
};


