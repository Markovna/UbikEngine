#include <array>

#include "window.h"
#include "base/log.h"
#include "GLFW/glfw3.h"

static void error_callback(int error_code, const char* description) {
  logger::core::Error("GLFW error ({1}): {0}", description, error_code);
}

window::window(vec2i size) : size_(size) {
  int status = glfwInit();
  assert(status); // TODO: assert macro
  glfwSetErrorCallback(&error_callback);

  // TODO: move gl version to config
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* win = glfwCreateWindow(size_.x, size_.y, "Ubik", NULL, NULL);
  assert(win); // TODO: assert macro
  handle_ = win;

  glfwMakeContextCurrent(win);

  int buffer_w, buffer_h; glfwGetFramebufferSize(win, &buffer_w, &buffer_h);
  resolution_ = { buffer_w, buffer_h };

  glfwSetWindowUserPointer(win, this);

  // set input callbacks
  glfwSetKeyCallback(win, [](GLFWwindow *w, int key, int scancode, int action, int mods) {
    window* window = (class window*) glfwGetWindowUserPointer(w);
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      window->push_event({
                            .type = event_type::KeyPress,
                            .key_press = key_press_event{
                                key_code(key),
                                bool(mods & GLFW_MOD_CONTROL),
                                bool(mods & GLFW_MOD_SHIFT),
                                bool(mods & GLFW_MOD_ALT),
                                bool(mods & GLFW_MOD_SUPER),
                                action == GLFW_REPEAT
                            }
                        });
    }
    else if (action == GLFW_RELEASE) {
      window->push_event({
                            .type = event_type::KeyRelease,
                            .key_release = key_release_event {
                                key_code(key),
                                bool(mods & GLFW_MOD_CONTROL),
                                bool(mods & GLFW_MOD_SHIFT),
                                bool(mods & GLFW_MOD_ALT),
                                bool(mods & GLFW_MOD_SUPER)
                            }
                        });
    }
  });

  glfwSetMouseButtonCallback(win, [](GLFWwindow *w, int button, int action, int mods) {
    window* window = (class window*)glfwGetWindowUserPointer(w);
    if (action == GLFW_PRESS) {
      window->push_event({ .type = event_type::MouseDown, .mouse_down = mouse_down_event{(mouse_code)button}});
    }
    else if (action == GLFW_RELEASE) {
      window->push_event({ .type = event_type::MouseUp, .mouse_up = mouse_up_event{(mouse_code)button}});
    }
  });

  glfwSetScrollCallback(win, [](GLFWwindow* w, double xoffset, double yoffset){
    window* window = (class window*)glfwGetWindowUserPointer(w);
    window->push_event({ .type = event_type::Scroll, .scroll = scroll_event { float(xoffset), float(yoffset) }});
  });

  glfwSetCursorPosCallback(win, [](GLFWwindow *w, double xpos, double ypos) {
    window* window = (class window*)glfwGetWindowUserPointer(w);
    window->push_event({ .type = event_type::MouseMove, .mouse_move = mouse_move_event { float(xpos), float(ypos) }});
  });

  glfwSetCharCallback(win, [](GLFWwindow* w, unsigned int codepoint) {
    window* window = (class window*)glfwGetWindowUserPointer(w);
    window->push_event({ .type = event_type::Text, .text = text_event { codepoint } });
  });

  glfwSetWindowCloseCallback(win, [](GLFWwindow *w) {
    window* window = (class window*)glfwGetWindowUserPointer(w);
    window->push_event({ .type = event_type::Close, .close = {}});
  });

  glfwSetWindowSizeCallback(win, [](GLFWwindow *w, int width, int height) {
    window* window = (class window*) glfwGetWindowUserPointer(w);
    window->size_ = { width, height };

    int buffer_w, buffer_h; glfwGetFramebufferSize(w, &buffer_w, &buffer_h);
    window->resolution_ = { buffer_w, buffer_h };

    window->push_event({.type = event_type::Resize, .resize = {.resolution = window->resolution_, .size = window->size_}});
  });
}

window::~window() {
  glfwDestroyWindow((GLFWwindow*)handle_);
  glfwTerminate();
}

bool window::poll_event(window_event& event) {
  if (events_.empty())
    return false;

  event = events_.front();
  events_.pop();
  return true;
}

void window::push_event(const window_event& event) {
  events_.emplace(event);
}

window::window_handle window::get_handle() const {
  return handle_;
}

void window::update() {
  glfwPollEvents();
}

GLFWcursor* to_glfw_cursor(cursor::type cursor_type) {
  static std::array<GLFWcursor*, cursor::Count> mouse_cursors = [](){
    std::array<GLFWcursor*, cursor::Count> cursors{};
    cursors[cursor::Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursors[cursor::Text_Input] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    cursors[cursor::Resize_Vertical] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    cursors[cursor::Resize_Horizontal] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    cursors[cursor::Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
#if GLFW_HAS_NEW_CURSORS
    cursors[Cursor::Resize_All] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
        cursors[Cursor::Resize_Top_Left_Bottom_Right] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
        cursors[Cursor::Resize_Bottom_Left_Top_Right] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
        cursors[Cursor::Not_Allowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
#else
    cursors[cursor::Resize_All] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursors[cursor::Resize_Top_Left_Bottom_Right] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursors[cursor::Resize_Bottom_Left_Top_Right] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    cursors[cursor::Not_Allowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif
    return cursors;
  }();

  return mouse_cursors[cursor_type];
}

void window::set_cursor(cursor::type cursor) {
  if (glfwGetInputMode((GLFWwindow*)handle_, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    return;

  if (cursor == cursor::None) {
    glfwSetInputMode((GLFWwindow*)handle_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  }
  else {
    glfwSetCursor((GLFWwindow*)handle_, to_glfw_cursor(cursor));
    glfwSetInputMode((GLFWwindow*)handle_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}
vec2i window::get_size() const {
  return size_;
}
vec2i window::get_resolution() const {
  return resolution_;
}
