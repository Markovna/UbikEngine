#include <core/assets/texture.h>
#include <core/assets/shader.h>
#include "resources.h"


namespace resources {

void init() {
  register_pool<texture>(::assets::loader::load<texture>);
  register_pool<shader>(::assets::loader::load<shader>);
}

void shutdown() {
  unregister_pool<texture>();
  unregister_pool<shader>();
}

compiler* g_compiler;

void init_compiler() {
  g_compiler = new compiler;
  g_compiler->register_compiler("texture", ::assets::compiler::compile<texture>);
  g_compiler->register_compiler("shader", ::assets::compiler::compile<shader>);
}

void shutdown_compiler() {
  delete g_compiler;
}

void compile_all_assets(const char *directory, assets::provider* provider) {
  std::vector<fs::path> files;

  for(auto it = fs::recursive_directory_iterator(directory); it != fs::recursive_directory_iterator(); ++it ) {
    if (it->path() == fs::paths::cache()) {
      it.disable_recursion_pending();
      continue;
    }

    if (fs::is_directory(it->path())) {
      continue;
    }

    if (it->path().extension() != ".meta") {
      continue;
    }

//    files.push_back(fs::append(it->path().parent_path(), it->path().stem()));
    files.push_back(it->path());
  }

  for (auto& file : files) {
    g_compiler->compile(file.c_str(), provider);
    logger::core::Info("{0} compiled.", file.c_str());
  }
}

}