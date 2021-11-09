cmake_minimum_required(VERSION 3.17)
project(engine)

set(CMAKE_CXX_STANDARD 17)

function(add_plugin name)
    cmake_parse_arguments(PARSE_ARGV 1 ADD_PLUGIN "" "" "FILES;LIBRARIES")
    add_library(${name} SHARED ${ADD_PLUGIN_FILES})
    target_link_libraries(${name} PRIVATE ${ADD_PLUGIN_LIBRARIES})
    set_target_properties(${name}
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${SANDBOX_LIBS_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${SANDBOX_LIBS_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${SANDBOX_LIBS_DIR}"
            )
endfunction()

add_subdirectory(thirdparty/glfw glfw)
add_subdirectory(thirdparty/glad glad)
add_subdirectory(thirdparty/spdlog spdlog)
add_subdirectory(thirdparty/stb_image stb_image)
add_subdirectory(thirdparty/efsw efsw)
add_subdirectory(thirdparty/imgui imgui)
add_subdirectory(thirdparty/glslang glslang)

set(BASE_SRC
        src/base/slot_map.h src/base/delegate.h src/base/event.h src/base/key_codes.h src/base/mouse_codes.h src/base/color.h src/base/color.cpp src/base/math.h src/base/math.cpp src/base/cursor.h src/base/iterator_range.h src/base/profiler.h src/base/profiler.cpp src/base/macro.h src/base/log.h src/base/log.cpp
        src/base/guid.cpp
        src/base/detector.h src/base/timer.cpp src/base/timer.h src/base/type_name.h src/base/memory.h src/base/allocator.cpp src/base/allocator.h src/base/flags.h src/base/crc32.h)

set(CORE_SRC
        src/core/application.cpp
        src/core/assets/assets.h
        src/core/assets/assets.cpp
        src/core/assets/resources.cpp src/core/assets/resources.h
        src/core/ecs.h src/core/ecs.cpp
        src/core/world.h src/core/renderer.cpp src/core/renderer.h
        src/core/input_system.cpp src/core/input_system.h
        src/core/assets/texture.cpp src/core/assets/texture.h src/core/assets/shader.cpp src/core/assets/shader.h src/core/components/mesh_component.cpp src/core/components/mesh_component.h src/core/components/camera_component.cpp src/core/components/camera_component.h src/core/world.cpp src/core/meta/registration.h src/core/meta/registration.cpp src/core/serialization.h src/core/meta/type.h src/core/meta/type_info.h src/core/meta/type_info.cpp src/core/meta/schema.cpp src/core/meta/schema.h src/core/assets/asset_loader.h src/core/assets/asset.h src/core/assets/asset.cpp
        src/core/render_texture.h src/core/render_texture.cpp
        src/core/plugins.cpp src/core/plugins.h src/core/simulation.cpp src/core/simulation.h src/core/assets/asset_compiler.h src/core/assets/filesystem_provider.cpp src/core/assets/filesystem_provider.h src/core/assets/resource_compiler.cpp src/core/assets/resource_compiler.h src/gfx/experimental/shader_repository.cpp src/gfx/experimental/shader_repository.h src/gfx/experimental/stream_buffers.cpp src/gfx/experimental/stream_buffers.h)

set(GFX_SRC
        src/gfx/gfx.cpp src/gfx/gfx.h src/gfx/gfx_config.h src/gfx/gfx_details.cpp src/gfx/gfx_details.h src/gfx/renderer_api.h src/gfx/renderer_api_gl.h src/gfx/renderer_api_gl.cpp
        src/gfx/experimental/gfx.h src/gfx/experimental/renderer.cpp src/gfx/experimental/renderer.h src/gfx/experimental/render_context.cpp src/gfx/experimental/render_context.h src/gfx/experimental/command_buffers.cpp src/gfx/experimental/command_buffers.h src/gfx/experimental/render_context_opengl.cpp src/gfx/experimental/render_context_opengl.h src/gfx/experimental/shader.cpp src/gfx/experimental/shader.h src/gfx/experimental/experimental.cpp src/gfx/experimental/vertex_layout_desc.cpp src/gfx/experimental/vertex_layout_desc.h src/gfx/experimental/shader_compiler.h src/gfx/experimental/shader_compiler_opengl.cpp src/gfx/experimental/shader_compiler_opengl.h src/gfx/experimental/asset_repository.cpp src/gfx/experimental/asset_repository.h)

set(PLATFORM_SRC
        src/platform/window.h src/platform/window.cpp src/base/window_event.h
        src/platform/os.cpp src/platform/os.h
        src/platform/file_system.h src/platform/file_system.cpp)

set(ENGINE_LIBS
        glfw
        glad
        spdlog
        stb_image
        glslang
        SPIRV
        )

set(ENGINE_INCLUDES src)

set(SANDBOX_LIBS_DIR ${CMAKE_SOURCE_DIR}/sandbox/.ubik/libs)

file(MAKE_DIRECTORY ${SANDBOX_LIBS_DIR})

# ----------------------------------------- #
# --------------- Game lib ---------------- #
# ----------------------------------------- #

add_library(game STATIC
        ${BASE_SRC} ${CORE_SRC} ${GFX_SRC} ${PLATFORM_SRC} src/core/game.cpp
        )

target_link_libraries(game PRIVATE ${ENGINE_LIBS})
target_include_directories(game PRIVATE ${ENGINE_INCLUDES})

# ----------------------------------------- #
# -------------- Engine lib --------------- #
# ----------------------------------------- #

add_library(engine SHARED
        ${BASE_SRC} ${CORE_SRC} ${GFX_SRC} ${PLATFORM_SRC}
        )

target_link_libraries(engine PUBLIC ${ENGINE_LIBS})
target_include_directories(engine PUBLIC ${ENGINE_INCLUDES})

# ----------------------------------------- #
# ----------- Sandbox executable ---------- #
# ----------------------------------------- #

set(SANDBOX_DIR ${CMAKE_SOURCE_DIR}/sandbox/)

add_executable(host
        ${SANDBOX_DIR}/src/load_libraries.cpp
        ${SANDBOX_DIR}/src/sandbox.cpp ${SANDBOX_DIR}/src/sandbox.h
        ${SANDBOX_DIR}/src/spin_plugin.cpp ${SANDBOX_DIR}/src/spin_plugin.h
        ${SANDBOX_DIR}/src/sandbox_plugin.cpp ${SANDBOX_DIR}/src/sandbox_plugin.h)

target_link_libraries(host PRIVATE game ${ENGINE_LIBS})
target_include_directories(host PRIVATE ${ENGINE_INCLUDES})

# ----------------------------------------- #
# ------------ Sandbox plugins ------------ #
# ----------------------------------------- #

add_plugin(sandbox
        FILES
        ${SANDBOX_DIR}/src/sandbox.cpp ${SANDBOX_DIR}/src/sandbox.h
        LIBRARIES engine)

add_plugin(spin_plugin
        FILES
        ${SANDBOX_DIR}/src/spin_plugin.cpp ${SANDBOX_DIR}/src/spin_plugin.h
        LIBRARIES engine)

add_plugin(sandbox_plugin
        FILES
        ${SANDBOX_DIR}/src/spin_plugin.cpp ${SANDBOX_DIR}/src/spin_plugin.h
        ${SANDBOX_DIR}/src/sandbox_plugin.cpp ${SANDBOX_DIR}/src/sandbox_plugin.h
        LIBRARIES engine)

# ----------------------------------------- #
# -------------- Editor lib --------------- #
# ----------------------------------------- #

add_library(editor SHARED
        src/platform/file_watcher.h src/platform/file_watcher.cpp
        src/editor/gui/imgui_renderer.cpp src/editor/gui/imgui_renderer.h
        src/editor/gui/gui.h src/editor/gui/gui.cpp
        src/editor/editor_gui.h src/editor/editor_gui.cpp)

target_link_libraries(editor PRIVATE engine)
target_link_libraries(editor PUBLIC efsw imgui)

target_include_directories(editor PUBLIC src/editor/)

# ----------------------------------------- #
# ------------ Editor plugins ------------- #
# ----------------------------------------- #

add_plugin(game_view_gui
        FILES src/editor/plugins/game_view_gui.cpp src/editor/plugins/game_view_gui.h
        LIBRARIES engine editor)

add_plugin(file_browser_gui
        FILES src/editor/plugins/file_browser_gui.cpp
        LIBRARIES engine editor)

add_plugin(entity_asset_editor
        FILES src/editor/plugins/entity_asset_editor.cpp
        LIBRARIES engine editor)

# ----------------------------------------- #
# ----------- Editor executable ----------- #
# ----------------------------------------- #

add_executable(ubik
        src/editor/editor.cpp
        src/editor/library_loader.cpp src/editor/library_loader.h)

target_link_libraries(ubik PRIVATE engine editor)

# TODO
list(APPEND UBIK_DEPENDENCIES game_view_gui entity_asset_editor file_browser_gui)
list(APPEND UBIK_DEPENDENCIES sandbox_plugin sandbox spin_plugin)
add_dependencies(ubik ${UBIK_DEPENDENCIES})

# ----------------------------------------- #

add_executable(experimental

        src/gfx/experimental/experimental.cpp

        src/gfx/experimental/render_context_opengl.cpp
        src/gfx/experimental/render_context.cpp
        src/gfx/experimental/renderer.cpp
        src/gfx/experimental/command_buffers.cpp
        src/gfx/experimental/vertex_layout_desc.cpp

        src/base/allocator.cpp
        src/base/log.cpp
        src/base/math.cpp
        src/platform/window.cpp
        )

target_link_libraries(experimental PRIVATE glfw glad spdlog stb_image)
target_include_directories(experimental PRIVATE src)


