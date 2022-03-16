#include "assets_filesystem.h"
#include "asset_repository.h"
#include "base/log.h"
#include "base/json.hpp"

#include <fstream>
#include <sstream>
#include <unordered_set>

static fs::path get_buffers_path(const fs::path& path) {
  return fs::concat(path, ".buffers");
}

void load_assets(
    const assets_filesystem& filesystem,
    asset_repository& repository,
    std::initializer_list<fs::path> extensions) {

  std::unordered_set<std::string> extensions_set { extensions.begin(), extensions.end() };
  for (auto it = fs::recursive_directory_iterator(fs::project_path());
            it != fs::recursive_directory_iterator();
            it++) {
    if (it->is_directory())
      continue;

    if (!extensions_set.empty() && !extensions_set.count(it->path().extension()))
      continue;

    fs::path relative = fs::relative(it->path(), fs::project_path());
    filesystem.load(repository, relative);
  }
}

void assets_filesystem::load(asset_repository& repository, const fs::path& path) const {
  fs::path fullpath = fs::to_project_path(path);
  std::ifstream file = fs::read_file(fullpath, std::ios::in);
  nlohmann::json j = nlohmann::json::parse(file);
  if (j.empty())
    return;

  guid guid = guid::from_string(j.at("__guid"));
  if (auto* asset = repository.get_asset(guid)) {
    repository.destroy_asset(asset->id());
  }

  asset& asset = parse_json(j, repository, get_buffers_path(fullpath));
  repository.set_asset_path(asset.id(), path);
}

void save_assets(assets_filesystem& filesystem, asset_repository& repository, bool remap_buffers) {
  for (auto& [asset, info] : repository) {
    filesystem.save(repository, info.path, remap_buffers);
  }
}

void assets_filesystem::save(asset_repository& repository, const fs::path& path, bool remap_buffers) {
  if (auto* asset = repository.get_asset_by_path(path)) {
    save(repository, *asset, path, remap_buffers);
  } else {
    logger::core::Error("Asset save failed: couldn't find asset with path {} in asset repository.", path.c_str());
  }
}

void assets_filesystem::save(asset_repository& repository, asset& asset, const fs::path& path, bool remap_buffers) {
  fs::path fullpath = fs::to_project_path(path);
  fs::path buffers_directory = get_buffers_path(fullpath);

  std::vector<buffer_id> buffers;
  nlohmann::json j = asset_to_json(asset, repository, buffers);

  std::ofstream file(fullpath, std::ios::trunc);
  file << j.dump(2);

  fs::assure(buffers_directory);

  std::unordered_set<std::string> buffer_paths;
  for (buffer_id& buf_id : buffers) {
    fs::path buf_path = fs::append(buffers_directory, std::to_string(repository.buffer_hash(buf_id)));
    buffer_paths.insert(buf_path);

    if (!fs::exists(buf_path)) {
      repository.write_buffer_to_file(buf_id, buf_path, 0);
      logger::core::Info("Saved buffer at path {}", buf_path.c_str());
    }

    if (remap_buffers) {
      repository.map_buffer_to_file(buf_id, buf_path, 0);
    }
  }

  std::vector<fs::path> paths_to_remove;
  auto dir_listing = fs::directory_iterator(buffers_directory);
  std::remove_copy_if(fs::begin(dir_listing), fs::end(dir_listing), std::back_inserter(paths_to_remove),
          [&](const fs::path& p) {
              return buffer_paths.count(p);
          });

  for (auto& p : paths_to_remove) {
    fs::remove(p);
  }

  logger::core::Info("Asset saved at path {}", path.c_str());
}
