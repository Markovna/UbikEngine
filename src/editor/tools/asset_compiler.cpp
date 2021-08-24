#include "asset_compiler.h"
#include "core/assets/texture.h"
#include "stb_image.h"

#include <iostream>

namespace assets {

template<>
bool compile_asset<texture>(std::ifstream& stream, const asset& settings, std::ostream& output) {

  stbi_set_flip_vertically_on_load(true);

  std::size_t size = stream.rdbuf()->pubseekoff(0, std::ios::end, std::ios_base::in);
  char* buffer = new char[size];
  stream.rdbuf()->pubseekpos(0, std::ios_base::in);
  stream.rdbuf()->sgetn(buffer, size);

  int32_t width, height, channels, desired_channels = 0;
  uint8_t* data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(buffer), size, &width, &height, &channels, desired_channels);

  output.write((char*) &width, sizeof(width));
  output.write((char*) &height, sizeof(height));
  output.write((char*) &channels, sizeof(channels));
  output.write((char*) data, width * height * channels);
  stbi_image_free(data);

  return true;
}

template<>
bool compile_asset<shader>(std::ifstream& stream, const asset& settings, std::ostream& output) {
  std::string source { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };

  static const std::string type_token = "#type";
  static const std::string vertex_type = "vertex";
  static const std::string fragment_type = "fragment";

  std::string vertex_shader, fragment_shader;
  gfx::attribute::binding_pack bindings;

  size_t pos = source.find(type_token, 0);
  std::string type;
  while (pos != std::string::npos) {
    size_t eol = source.find_first_of('\n', pos);
    size_t begin = pos + type_token.size() + 1;
    type = source.substr(begin, eol - begin);
    uint64_t nextLinePos = eol + 1;
    pos = source.find(type_token, nextLinePos);

    if (type == vertex_type) {
      vertex_shader = source.substr(nextLinePos, pos == std::string::npos ? pos : pos - nextLinePos);
    } else if (type == fragment_type) {
      fragment_shader = source.substr(nextLinePos, pos == std::string::npos ? pos : pos - nextLinePos);
    }
  }

  {
    static const std::string token = "#binding";
    size_t pos = vertex_shader.find(token, 0);
    int loc = 0;
    const char* location_template_str = "layout (location = #)";
    const size_t location_str_pos = 19;
    while (pos != std::string::npos) {
      size_t eol = vertex_shader.find_first_of('\n', pos);
      size_t begin = pos + token.size() + 1;
      std::string binding_type = vertex_shader.substr(begin, eol - begin);

      std::string location_str(location_template_str);
      location_str.replace(location_str_pos, 1, std::to_string(loc));
      vertex_shader.replace(pos, eol - pos, location_str);

      gfx::attribute::binding::type binding;
      if (gfx::attribute::binding::try_parse(binding_type, binding)) {
        gfx::attribute::set_pack(bindings, binding, loc);
      } else {
        logger::core::Error("Can't parse {}", binding_type);
      }

      pos = vertex_shader.find(token, pos);
      loc++;
    }
  }

  size_t vertex_size = vertex_shader.size() + 1;
  size_t fragment_size = fragment_shader.size() + 1;
  output.write((char*) &bindings, sizeof(bindings));

  output.write((char*) &vertex_size, sizeof(vertex_size));
  output.write(vertex_shader.c_str(), vertex_size);

  output.write((char*) &fragment_size, sizeof(fragment_size));
  output.write(fragment_shader.c_str(), fragment_size);
  return true;
}

void compile_asset(const char *path)  {
  fs::path absolute_path = fs::absolute(path);

  asset meta = assets::read(fs::concat(absolute_path, ".meta"));

  std::string guid;
  assets::get(meta, "__guid", guid);

  std::string type;
  assets::get(meta, "type", type);

  fs::path output_path = fs::append(fs::paths::import(), guid);

  asset asset;
  assets::set(asset, "__guid", guid);
  assets::set(asset, "type", type);
  assets::set(asset, "path", path);
  assets::write(asset, fs::concat(output_path, ".asset"));

  std::ifstream stream = fs::read_file(absolute_path);
  std::ofstream out_stream(output_path, std::ios::binary | std::ios::trunc);

  if (type == "texture") {
    assets::compile_asset<texture>(
        stream,
        meta,
        out_stream
    );
  } else if (type == "shader") {
    assets::compile_asset<shader>(
        stream,
        meta,
        out_stream
    );
  }
}

void compile_all_assets(const char *directory) {
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


    files.push_back(fs::append(it->path().parent_path(), it->path().stem()));
  }

  for (auto& file : files) {
    compile_asset(file.c_str());
  }
}


}