#include "imgui_renderer.h"
#include "gui.h"
#include "platform/window.h"
#include "core/assets/shader.h"
#include "core/assets/texture.h"
#include "core/input_system.h"

std::unique_ptr<gui_renderer> gui_renderer::create(window* w, assets::provider* provider) {
  return std::make_unique<gui_renderer>(w, provider);
}

static cursor::type cursor_type(ImGuiMouseCursor cursor) {
  using CursorsMap = std::array<cursor::type, cursor::Count>;
  const static CursorsMap cursors_map = [](){
    CursorsMap map;
    map[ImGuiMouseCursor_Arrow] = cursor::Arrow;
    map[ImGuiMouseCursor_ResizeAll] = cursor::Resize_All;
    map[ImGuiMouseCursor_TextInput] = cursor::Text_Input;
    map[ImGuiMouseCursor_ResizeNS] = cursor::Resize_Vertical;
    map[ImGuiMouseCursor_ResizeEW] = cursor::Resize_Horizontal;
    map[ImGuiMouseCursor_ResizeNESW] = cursor::Resize_Bottom_Left_Top_Right;
    map[ImGuiMouseCursor_ResizeNWSE] = cursor::Resize_Top_Left_Bottom_Right;
    map[ImGuiMouseCursor_Hand] = cursor::Hand;
    map[ImGuiMouseCursor_NotAllowed] = cursor::Not_Allowed;
    return map;
  }();
  if (cursor >= 0) {
    return cursors_map[cursor];
  }
  return cursor::None;
}

static void setup_keymap(ImGuiIO &io) {
  // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
  io.KeyMap[ImGuiKey_Tab] = key::Tab;
  io.KeyMap[ImGuiKey_LeftArrow] = key::Left;
  io.KeyMap[ImGuiKey_RightArrow] = key::Right;
  io.KeyMap[ImGuiKey_UpArrow] = key::Up;
  io.KeyMap[ImGuiKey_DownArrow] = key::Down;
  io.KeyMap[ImGuiKey_PageUp] = key::PageUp;
  io.KeyMap[ImGuiKey_PageDown] = key::PageDown;
  io.KeyMap[ImGuiKey_Home] = key::Home;
  io.KeyMap[ImGuiKey_End] = key::End;
  io.KeyMap[ImGuiKey_Insert] = key::Insert;
  io.KeyMap[ImGuiKey_Delete] = key::Delete;
  io.KeyMap[ImGuiKey_Backspace] = key::Backspace;
  io.KeyMap[ImGuiKey_Space] = key::Space;
  io.KeyMap[ImGuiKey_Enter] = key::Enter;
  io.KeyMap[ImGuiKey_Escape] = key::Escape;
  io.KeyMap[ImGuiKey_KeyPadEnter] = key::KPEnter;
  io.KeyMap[ImGuiKey_A] = key::A;
  io.KeyMap[ImGuiKey_C] = key::C;
  io.KeyMap[ImGuiKey_V] = key::V;
  io.KeyMap[ImGuiKey_X] = key::X;
  io.KeyMap[ImGuiKey_Y] = key::Y;
  io.KeyMap[ImGuiKey_Z] = key::Z;
}

static void setup_style() {
  ImGui::StyleColorsDark();

  ImVec4* colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_WindowBg]               = ImVec4(0.16f, 0.16f, 0.16f, 0.94f);
  colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
  colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
  colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
  colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.11f, 0.11f, 0.11f, 0.94f);
  colors[ImGuiCol_FrameBgActive]          = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
  colors[ImGuiCol_TitleBg]                = ImVec4(0.16f, 0.16f, 0.16f, 0.94f);
  colors[ImGuiCol_TitleBgActive]          = ImVec4(0.10f, 0.15f, 0.18f, 0.94f);
  colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
  colors[ImGuiCol_MenuBarBg]              = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
  colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.16f, 0.17f, 0.18f, 0.00f);
  colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
  colors[ImGuiCol_CheckMark]              = ImVec4(0.39f, 0.42f, 0.44f, 0.94f);
  colors[ImGuiCol_SliderGrab]             = ImVec4(0.29f, 0.32f, 0.33f, 0.94f);
  colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.35f, 0.38f, 0.40f, 0.94f);
  colors[ImGuiCol_Button]                 = ImVec4(0.24f, 0.26f, 0.27f, 0.94f);
  colors[ImGuiCol_ButtonHovered]          = ImVec4(0.29f, 0.32f, 0.33f, 0.94f);
  colors[ImGuiCol_ButtonActive]           = ImVec4(0.31f, 0.35f, 0.36f, 0.94f);
  colors[ImGuiCol_Header]                 = ImVec4(0.24f, 0.26f, 0.27f, 0.94f);
  colors[ImGuiCol_HeaderHovered]          = ImVec4(0.30f, 0.32f, 0.33f, 0.94f);
  colors[ImGuiCol_HeaderActive]           = ImVec4(0.32f, 0.34f, 0.35f, 0.94f);
  colors[ImGuiCol_Separator]              = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
  colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.10f, 0.10f, 0.94f);
  colors[ImGuiCol_SeparatorActive]        = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
  colors[ImGuiCol_ResizeGrip]             = ImVec4(0.24f, 0.26f, 0.27f, 0.94f);
  colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.29f, 0.32f, 0.33f, 0.94f);
  colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.35f, 0.38f, 0.40f, 0.94f);
  colors[ImGuiCol_Tab]                    = ImVec4(0.16f, 0.42f, 0.66f, 0.50f);
  colors[ImGuiCol_TabHovered]             = ImVec4(0.16f, 0.42f, 0.66f, 0.94f);
  colors[ImGuiCol_TabActive]              = ImVec4(0.16f, 0.42f, 0.66f, 0.94f);
  colors[ImGuiCol_TabUnfocused]           = ImVec4(0.27f, 0.29f, 0.30f, 0.52f);
  colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.27f, 0.29f, 0.30f, 0.94f);
  colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
  colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
  colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

  ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_Right;
  ImGui::GetStyle().TabRounding = 4.0f;
  ImGui::GetStyle().WindowRounding = 0.0f;
  ImGui::GetStyle().AntiAliasedFill = true;
  ImGui::GetStyle().AntiAliasedLines = true;
  ImGui::GetStyle().AntiAliasedLinesUseTex = true;
  ImGui::GetStyle().WindowPadding = {4.0f, 6.0f};
}

//TODO: reserve imgui camera id
static const uint32_t camera_id = gfx::static_config::kViewsCapacity - 1;

void gui_renderer::on_resize(resize_event& event) {
  ImGuiIO& io = gui::GetIO();

  io.DisplaySize = ImVec2(event.size.x, event.size.y);
  io.DisplayFramebufferScale = { (float) event.resolution.x / io.DisplaySize.x, (float) event.resolution.y / io.DisplaySize.y };

  gfx::set_projection(camera_id, mat4::ortho(0, io.DisplaySize.x, 0, io.DisplaySize.y,0,1));
  gfx::set_view_rect(camera_id, {0, 0, event.resolution.x, event.resolution.y });
}

void gui_renderer::on_key_pressed(key_press_event &event) {
  ImGuiIO& io = ImGui::GetIO();
  io.KeysDown[event.key_code] = true;
  io.KeyCtrl = event.control;
  io.KeyShift = event.shift;
  io.KeyAlt = event.alt;
  io.KeySuper = event.super;
}

void gui_renderer::on_key_released(key_release_event &event) {
  ImGuiIO& io = ImGui::GetIO();
  io.KeysDown[event.key_code] = false;
  io.KeyCtrl = event.control;
  io.KeyShift = event.shift;
  io.KeyAlt = event.alt;
  io.KeySuper = event.super;
}

void gui_renderer::on_mouse_down(mouse_down_event &e) {
  ImGuiIO& io = ImGui::GetIO();
  io.MouseDown[e.mouse_code] = true;
}

void gui_renderer::on_mouse_up(mouse_up_event &e) {
  ImGuiIO& io = ImGui::GetIO();
  io.MouseDown[e.mouse_code] = false;
}

void gui_renderer::on_mouse_move(mouse_move_event &e) {
  ImGuiIO& io = ImGui::GetIO();
  io.MousePos.x = e.x;
  io.MousePos.y = e.y;
}

void gui_renderer::on_scroll(scroll_event &e) {
  ImGuiIO& io = ImGui::GetIO();
  io.MouseWheel += e.y;
}

void gui_renderer::on_text_input(text_event &e) {
  ImGuiIO& io = ImGui::GetIO();
  if(e.unicode > 0 && e.unicode < 0x10000) {
    io.AddInputCharacter(e.unicode);
  }
}

gui_renderer::gui_renderer(window* w, assets::provider* provider) : context_(gui::CreateContext()) {

  vb_handle_ = gfx::create_vertex_buffer(gfx::make_ref(nullptr, kBufferMaxSize * sizeof(ImDrawVert)), kBufferMaxSize, {
      { gfx::attribute::binding::Position, gfx::attribute::format::Float2() },
      { gfx::attribute::binding::TexCoord0, gfx::attribute::format::Float2() } ,
      {gfx::attribute::binding::Color0, gfx::attribute::format::Byte4(), true }
  });

  ib_handle_ = gfx::create_index_buffer(gfx::make_ref(nullptr, kBufferMaxSize * sizeof(ImDrawIdx)), kBufferMaxSize);
  texture_uniform_handle_ = gfx::create_uniform("Texture");


  ImGuiIO &io = ImGui::GetIO();

  // Build texture atlas
  uint8_t* pixels;
  int width, height;
  // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small)
  // because it is more likely to be compatible with user's existing shaders. If
  // your ImTextureId represent a higher-level concept than just a GL texture id,
  // consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  fonts_texture_ = std::make_unique<texture>(pixels, width, height, gfx::texture_format::RGBA8);
  shader_ = resources::load<shader>(fs::absolute("assets/shaders/GUIShader.shader"), provider);

  io.Fonts->TexID = (ImTextureID)(intptr_t) fonts_texture_->handle().id;

  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
//    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // TODO

  vec2i resolution = w->get_resolution();
  io.DisplaySize = ImVec2(w->get_size().x, w->get_size().y);
  io.DisplayFramebufferScale = { (float) resolution.x / io.DisplaySize.x, (float) resolution.y / io.DisplaySize.y };

  gfx::set_view(camera_id, mat4::identity());
  gfx::set_clear(camera_id, gfx::clear_flag::Color | gfx::clear_flag::Depth);
  gfx::set_clear_color(camera_id, color {0.1, 0.1, 0.1, 1.0});
  gfx::set_projection(camera_id, mat4::ortho(0, io.DisplaySize.x, 0, io.DisplaySize.y,0,1));
  gfx::set_view_rect(camera_id, {0, 0, resolution.x, resolution.y });

  w->set_cursor(cursor());

  setup_keymap(io);
  setup_style();
}

void gui_renderer::begin_frame() {
  ImGuiIO& io = gui::GetIO();

  io.DeltaTime = timer_.restart().as_seconds();

  gui::NewFrame();
}

void gui_renderer::end_frame() {
  gui::Render();
  render(gui::GetDrawData());
}

void gui_renderer::render(ImDrawData *draw_data) {
  uint32_t vertices_offset = 0;
  uint32_t indices_offset = 0;

  ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
  ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    uint32_t num_vertices = cmd_list->VtxBuffer.size();
    uint32_t num_indices = cmd_list->IdxBuffer.size();

    gfx::update_vertex_buffer(
         vb_handle_,
        gfx::make_ref((void*)cmd_list->VtxBuffer.begin(), num_vertices * sizeof(ImDrawVert)),
        vertices_offset * sizeof(ImDrawVert)
    );
    gfx::update_index_buffer(
        ib_handle_,
        gfx::make_ref((void*)cmd_list->IdxBuffer.begin(), num_indices * sizeof(ImDrawIdx)),
        indices_offset * sizeof(ImDrawIdx)
    );

    std::uint32_t offset = 0;
    for (const ImDrawCmd *cmd = cmd_list->CmdBuffer.begin(), *cmdEnd = cmd_list->CmdBuffer.end();
         cmd != cmdEnd;
         ++cmd)
    {
      if (cmd->UserCallback) {
        cmd->UserCallback(cmd_list, cmd);
      }
      else if (cmd->ElemCount) {
        const std::uint16_t x = (cmd->ClipRect.x - clip_off.x) * clip_scale.x;
        const std::uint16_t y = (cmd->ClipRect.y - clip_off.y) * clip_scale.y;
        const std::uint16_t width  = (cmd->ClipRect.z - clip_off.x) * clip_scale.x - x;
        const std::uint16_t height = (cmd->ClipRect.w - clip_off.y) * clip_scale.y - y;

        auto texture = gfx::texture_handle{(uint32_t)(intptr_t)cmd->TextureId};
        gfx::set_scissor({ x, y, width, height });
        gfx::set_options(gfx::options::None);
        gfx::set_uniform(texture_uniform_handle_, texture, 0);
        gfx::set_buffer(vb_handle_, vertices_offset, num_vertices);
        gfx::set_buffer(ib_handle_, indices_offset + offset, cmd->ElemCount);

        gfx::render(camera_id, shader_->handle());
      }

      offset += cmd->ElemCount;
    }

    vertices_offset += num_vertices;
    indices_offset += num_indices;
  }
}

gui_renderer::~gui_renderer() {
  ImGuiIO &io = ImGui::GetIO();
  io.Fonts->TexID = nullptr;

  gui::DestroyContext(context_);

  if (vb_handle_)
    gfx::destroy(vb_handle_);

  if (ib_handle_)
    gfx::destroy(ib_handle_);

  if (texture_uniform_handle_)
    gfx::destroy(texture_uniform_handle_);
}

cursor::type gui_renderer::cursor() const {
  ImGuiIO& io = gui::GetIO();
  if (!(io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)) {
    static ImGuiMouseCursor last_cursor_type = gui::GetMouseCursor();
    ImGuiMouseCursor curr_cursor_type = gui::GetMouseCursor();
    if (last_cursor_type != curr_cursor_type) {
      cursor_ = cursor_type(curr_cursor_type);
      last_cursor_type = curr_cursor_type;
    }
  }

  return cursor_;
}

void gui_renderer::set_context() {
  gui::SetCurrentContext(context_);
}

void connect_gui_events(gui_renderer* gui_renderer, class input_system* input_events) {
  input_events->on_resize.connect(gui_renderer, &gui_renderer::on_resize);
  input_events->on_key_press.connect(gui_renderer, &gui_renderer::on_key_pressed);
  input_events->on_key_release.connect(gui_renderer, &gui_renderer::on_key_released);
  input_events->on_mouse_down.connect(gui_renderer, &gui_renderer::on_mouse_down);
  input_events->on_mouse_move.connect(gui_renderer, &gui_renderer::on_mouse_move);
  input_events->on_mouse_up.connect(gui_renderer, &gui_renderer::on_mouse_up);
  input_events->on_scroll.connect(gui_renderer, &gui_renderer::on_scroll);
  input_events->on_text.connect(gui_renderer, &gui_renderer::on_text_input);
}

void disconnect_gui_events(gui_renderer* gui_renderer, class input_system* input_events) {
  input_events->on_resize.disconnect(gui_renderer, &gui_renderer::on_resize);
  input_events->on_key_press.disconnect(gui_renderer, &gui_renderer::on_key_pressed);
  input_events->on_key_release.disconnect(gui_renderer, &gui_renderer::on_key_released);
  input_events->on_mouse_down.disconnect(gui_renderer, &gui_renderer::on_mouse_down);
  input_events->on_mouse_move.disconnect(gui_renderer, &gui_renderer::on_mouse_move);
  input_events->on_mouse_up.disconnect(gui_renderer, &gui_renderer::on_mouse_up);
  input_events->on_scroll.disconnect(gui_renderer, &gui_renderer::on_scroll);
  input_events->on_text.disconnect(gui_renderer, &gui_renderer::on_text_input);
}
