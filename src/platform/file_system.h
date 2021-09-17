#pragma once

#include <filesystem>

namespace fs {

using namespace std::filesystem;

namespace details {

struct config {
  path project_path;
  path cache_path;
};

config& get_config();

}

namespace paths {

const path& project();
const path& cache();

void project(const path&);

}

std::ifstream read_file(const fs::path &path, std::ios::openmode mode = std::ios::in | std::ios::binary);
std::ifstream read_file(const char* path, std::ios::openmode mode = std::ios::in | std::ios::binary);

path absolute(const char*);
path absolute(const path&);

path concat(const path&, const path&);
path concat(const path&, const char*);

path append(const path&, const path&);
path append(const path&, const char*);

}