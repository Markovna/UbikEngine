#pragma once

#include "core/assets/assets.h"

class filesystem_provider : public assets::provider {
 public:
  explicit filesystem_provider(const fs::path&);

  asset load(const fs::path&) override;
  fs::path get_path(const guid& id) override;
  void load_buffer(const fs::path&, uint64_t buffer_id, std::ostream&) override;

  void save(const fs::path&, const asset&);
  void reload(assets::repository&);

  static void save_buffer(const fs::path&, uint64_t buffer_id, const std::istream&);
  static void remove_buffer(const fs::path&, uint64_t buffer_id);

 private:
  static fs::path get_or_create_buffers_dir(const fs::path&);

 private:
  std::unordered_map<guid, std::string> guid_to_path_;
};

extern filesystem_provider* g_fsprovider;

void init_filesystem_provider();
void shutdown_filesystem_provider();
