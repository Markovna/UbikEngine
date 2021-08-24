#include "resource_compiler.h"

#include "core/assets/shader.h"
#include "core/assets/texture.h"
#include "stb_image.h"

#include <vector>
#include <unordered_set>
#include <sstream>
#include <random>

namespace experimental2::resources {

template<>
bool compile_asset<texture>(std::ifstream& resource, const assets::asset& settings, std::ostream& output) {
  stbi_set_flip_vertically_on_load(true);

  std::size_t size = resource.rdbuf()->pubseekoff(0, std::ios::end, std::ios_base::in);
  char* buffer = new char[size];
  resource.rdbuf()->pubseekpos(0, std::ios_base::in);
  resource.rdbuf()->sgetn(buffer, size);

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
bool compile_asset<shader>(std::ifstream& resource, const assets::asset& settings, std::ostream& output) {
  std::string source { std::istreambuf_iterator<char>(resource), std::istreambuf_iterator<char>() };

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

bool compile_asset_impl(const std::string& type, std::ifstream& stream, const assets::asset& settings, std::ostream& output) {
  using func_t = bool(*)(std::ifstream& stream, const assets::asset& settings, std::ostream&);
  static std::unordered_map<std::string, func_t> func_ptr = {
      { "shader", compile_asset<shader> },
      { "texture", compile_asset<texture> }
  };
  return func_ptr[type](stream, settings, output);
}


void compile_asset(const fs::path &path, const fs::path& directory) {
  logger::core::Info("Start compiling resource {}.", path.c_str());

  assets::asset meta;
  if (!assets::read(fs::absolute(path), meta))
    return;

  fs::path res_path = meta["source_path"];
  std::string res_id = meta["source_guid"];

  std::ifstream file = fs::read_file(fs::absolute(res_path));
  fs::path asset_path = fs::append(directory, res_id);

  fs::path buffers_dir = fs::concat(asset_path, ".buffers");
  if (fs::exists(buffers_dir)) {
    fs::remove_all(buffers_dir);
  } else {
    fs::create_directory(buffers_dir);
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint64_t> dis;
  uint64_t buffer_id = dis(gen);
  fs::path buffer_path = fs::append(buffers_dir, std::to_string(buffer_id));

  std::ofstream buffer(buffer_path, std::ios::binary | std::ios::trunc);
  compile_asset_impl(meta["type"], file, meta, buffer);

  assets::asset resource;
  resource["__guid"] = res_id;
  resource["source_path"] = res_path;
  resource["buffer"] = buffer_id;

  assets::write(resource, fs::append(directory, res_id));
}

void compile(
    const fs::path& directory,
    const fs::path& dest,
    std::initializer_list<fs::path> ignore) {

  std::vector<fs::path> files;
  std::set<fs::path> ignore_set { ignore };

  for (auto it = fs::recursive_directory_iterator(directory); it != fs::recursive_directory_iterator(); ++it ) {

    if (ignore_set.count(it->path())) {
      it.disable_recursion_pending();
      continue;
    }

    if (fs::is_directory(it->path())) {
      continue;
    }

    if (it->path().extension() != ".meta") {
      continue;
    }

    files.push_back(it->path());
  }

  for (auto& file : files) {
    compile_asset(file.c_str(), dest);
  }
}

}