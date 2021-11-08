#include "imgui.h"
#include <string_view>

namespace gui {

using namespace ImGui;

void begin_dockspace();
void image(std::string_view icon_id, ImVec2 size);

}