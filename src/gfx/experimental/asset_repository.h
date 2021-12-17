#pragma once

#include "base/slot_map.h"
#include "platform/file_system.h"
#include "base/guid.h"
#include "base/json.hpp"
#include "base/log.h"
#include "gfx/experimental/stream_buffers.h"
#include "shader_repository.h"

using asset = nlohmann::json;

enum class visit_recursive_result {
  BREAK,
  CONTINUE,
  RECURSE
};

template<class Iterator, class Operation>
int32_t visit_recursive(Iterator first, Iterator last, Operation op) {
  for (; first != last; ++first) {
    visit_recursive_result op_result = op(*first);
    if (op_result == visit_recursive_result::BREAK)
      return -1;

    if (op_result == visit_recursive_result::CONTINUE)
      continue;

    assert(op_result == visit_recursive_result::RECURSE);
    if (first->is_structured()) {
      if (auto visit_result = visit_recursive(first->begin(), first->end(), op))
        return visit_result;
    }
  }
  return 0;
}

bool read(const fs::path &path, asset &data);
void write(const fs::path &path, const asset &asset);

class assets_repository;

class asset_handle {
 public:
  using key_t = stdext::slot_map<struct any>::key_type;

  asset_handle() noexcept : repository_(nullptr), key_() {}
  asset_handle(assets_repository*, const key_t&);

  asset_handle(const asset_handle&);
  asset_handle(asset_handle&&) noexcept;

  asset_handle& operator=(const asset_handle&);
  asset_handle& operator=(asset_handle&&) noexcept;

  ~asset_handle();

  explicit operator bool() const noexcept;

  asset* operator->();
  asset& operator*();

  const asset* operator->() const;
  const asset& operator*() const;

  const fs::path& path();
  const guid& id();

  void swap(asset_handle& other) noexcept;

 private:
  assets_repository* repository_;
  key_t key_;
};

class assets_repository {
 private:
  struct asset_info {
    asset_info(asset a, fs::path p, guid id)
      : asset(std::move(a))
      , path(std::move(p))
      , guid(std::move(id))
    {}

    asset asset;
    fs::path path;
    guid guid;
  };

  using container_t = stdext::slot_map<asset_info>;

 public:
  template<class It>
  class basic_iterator {
   public:
    explicit basic_iterator(It it) : curr_(it) {}

    inline basic_iterator& operator++() {
      ++curr_;
      return *this;
    }

    inline basic_iterator operator++(int) {
      basic_iterator orig = *this;
      return ++(*this), orig;
    }

    inline basic_iterator& operator--() {
      --curr_;
      return *this;
    }

    inline basic_iterator operator--(int) {
      basic_iterator orig = *this;
      return --(*this), orig;
    }

    [[nodiscard]] inline bool operator==(const basic_iterator& other) const {
      return other.curr_ == curr_;
    }

    [[nodiscard]] inline bool operator!=(const basic_iterator& other) const {
      return !(*this == other);
    }

    [[nodiscard]] inline auto operator*() const {
      return std::tie(curr_->asset, curr_->path, curr_->guid);
    }

   private:
    It curr_;
  };

  using iterator = basic_iterator<container_t::iterator>;
  using const_iterator = basic_iterator<container_t::const_iterator>;

 public:
  asset_handle load(const fs::path&);
  asset_handle load(guid);

  asset_handle create_asset(guid, const fs::path&);
  void remove_asset(const fs::path&);
  void remove_asset(guid);
  void emplace(guid, const fs::path&, asset);

  iterator begin() { return iterator(storage_.begin()); };
  iterator end() { return iterator(storage_.end()); };

  [[nodiscard]] const_iterator begin() const { return const_iterator(storage_.begin()); };
  [[nodiscard]] const_iterator end() const { return const_iterator(storage_.end()); };

  stream_buffers& buffers() { return buffers_; }
  const stream_buffers& buffers() const { return buffers_; }

  void save_buffer(uint64_t id);

 private:
  friend class asset_handle;

  asset& get(asset_handle::key_t);
  [[nodiscard]] const asset& get(asset_handle::key_t) const;

  const fs::path& path(asset_handle::key_t);
  const guid& id(asset_handle::key_t);

  asset* find(asset_handle::key_t);
  [[nodiscard]] const asset* find(asset_handle::key_t) const;

  void inc_use_count(asset_handle::key_t);
  void dec_use_count(asset_handle::key_t);

 private:
  container_t storage_;
  std::unordered_map<guid, asset_handle::key_t> id_index_;
  std::unordered_map<std::string, asset_handle::key_t> path_index_;
  stream_buffers buffers_;
};

class assets_database {
 public:
  void open(fs::path);
  void load(assets_repository*);
  void close();
};

class assets_filesystem {
 public:
  assets_filesystem() noexcept = default;

  void load_assets(assets_repository&, std::initializer_list<fs::path> extensions = {}) const;
  void save_assets(assets_repository&);
  void save(assets_repository&, const fs::path&);
  void load(assets_repository&, const fs::path&) const;

 private:
  static fs::path get_buffers_path(const fs::path& path);
  void save(assets_repository&, const asset&, const fs::path&);
};
