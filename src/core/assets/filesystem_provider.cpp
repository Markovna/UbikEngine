#include "filesystem_provider.h"

asset filesystem_provider::load(const fs::path& path) {
  asset asset;
  assets::read(path, asset);
  return asset;
}

fs::path filesystem_provider::get_path(const guid& id) {
  auto iterator = guid_to_path_.find(id);
  if (iterator == guid_to_path_.end())
    return {};

  return iterator->second;
}

void filesystem_provider::save(const fs::path& path, const asset& asset) {
  guid id = guid::from_string(asset["__guid"]);
  guid_to_path_[id] = path;
  assets::write(path, asset);
}

void filesystem_provider::reload(assets::repository& repository) {
  // TODO: check for all changes and load it to repository
//  repository.replace()
}

void filesystem_provider::load_buffer(const fs::path& path, uint64_t buffer_id, std::ostream& stream) {
  fs::path buffer_path = fs::append(get_or_create_buffers_dir(path), std::to_string(buffer_id));
  std::ifstream file = fs::read_file(buffer_path);
  stream << file.rdbuf();
}

filesystem_provider::filesystem_provider(const fs::path& path) {
  for (auto it = fs::recursive_directory_iterator(path); it != fs::recursive_directory_iterator(); it++) {
    if (it->is_directory())
      continue;

    // TODO: check extension

    asset asset;
    if (!assets::read(it->path(), asset))
      continue;

    guid id = guid::from_string(asset["__guid"]);
    if (id.is_valid()) {
      guid_to_path_[id] = it->path();
    }
  }
}

void filesystem_provider::save_buffer(const fs::path& path, uint64_t buffer_id, const std::istream& buffer) {
  logger::core::Info("filesystem_provider::save_buffer {0}:{1}", path.c_str(), buffer_id);
  fs::path buffer_path = fs::append(get_or_create_buffers_dir(path), std::to_string(buffer_id));
  std::ofstream file(buffer_path, std::ios::binary | std::ios::trunc);
  file << buffer.rdbuf();
}

void filesystem_provider::remove_buffer(const fs::path& path, uint64_t buffer_id) {
  fs::path buffer_path = fs::append(get_or_create_buffers_dir(path), std::to_string(buffer_id));
  fs::remove(buffer_path);
}

fs::path filesystem_provider::get_or_create_buffers_dir(const fs::path& path) {
  fs::path buffers_path = fs::concat(path, ".buffers");
  if (!fs::exists(buffers_path)) {
    fs::create_directory(buffers_path);
  }
  return buffers_path;
}

filesystem_provider* g_fsprovider;

void init_filesystem_provider() {
  g_fsprovider = new filesystem_provider(fs::paths::project());
}

void shutdown_filesystem_provider() {
  delete g_fsprovider;
}