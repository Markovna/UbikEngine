#include "texture_compiler.h"
#include "texture.h"

void compile_textures(
  const std::unordered_set<std::string>& extensions,
  assets_repository& repository,
  assets_filesystem& assets)
{
  for (auto it = fs::recursive_directory_iterator(fs::current_path()); it != fs::recursive_directory_iterator(); it++) {
    if (it->is_directory())
      continue;

    if (!extensions.count(it->path().extension()))
      continue;

    fs::path p = it->path().lexically_relative(fs::current_path());
    compile_texture(
        it->path(),
        fs::concat(p, ".meta"),
        repository,
        assets
      );
  }
}