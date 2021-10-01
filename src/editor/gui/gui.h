#include "imgui.h"
#include <string_view>

namespace gui {

//struct image_desc {
//  ImTextureID texture_id;
//  ImVec2 uv0;
//  ImVec2 uv1;
//};

//class gui_atlas {
// private:
//  std::unordered_map<std::string, image_desc> image_descs_;
//
//
// public:
//  image_desc get(std::string_view);
//};

using namespace ImGui;

void begin_dockspace();
void image(std::string_view icon_id, ImVec2 size);

}