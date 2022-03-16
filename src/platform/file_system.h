#pragma once

#include <filesystem>

namespace fs {

using namespace std::filesystem;

void assure(const fs::path&);

std::ifstream read_file(const fs::path &path, std::ios::openmode mode = std::ios::in | std::ios::binary);
std::ifstream read_file(const char* path, std::ios::openmode mode = std::ios::in | std::ios::binary);

path concat(const path&, const path&);
path concat(const path&, const char*);

path append(const path&, const path&);
path append(const path&, const char*);

path to_project_path(const fs::path& p);

path project_path();
void project_path(const fs::path&);

}