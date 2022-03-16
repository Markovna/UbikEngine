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
//  using nlohmann::json;
  fs::path fullpath = fs::to_project_path(path);
  std::ifstream file = fs::read_file(fullpath, std::ios::in);
  nlohmann::json j = nlohmann::json::parse(file);
  if (j.empty())
    return;

  asset& asset = parse_json(j, repository, get_buffers_path(fullpath));
  repository.set_asset_path(asset.id(), path);
  if (j.contains("__guid"))
    repository.set_asset_guid(asset.id(), guid::from_string(j["__guid"]));
}

void save_assets(assets_filesystem& filesystem, asset_repository& repository) {
  for (auto& [asset, info] : repository) {
    filesystem.save(repository, info.path);
  }
}

void assets_filesystem::save(asset_repository& repository, const fs::path& path) {
  save(repository, *repository.get_asset(path), path);
}

void assets_filesystem::save(asset_repository& repository, asset& asset, const fs::path& path) {
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
      std::ofstream dst(buf_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
      auto buf = repository.load_buffer(buf_id);
      std::copy_n(buf.data(), buf.size(), std::ostreambuf_iterator<char>(dst));

      logger::core::Info("Saved buffer at path {}", buf_path.c_str());
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
}
