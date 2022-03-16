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

add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/glad)
add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/stb_image)
add_subdirectory(thirdparty/imgui)
add_subdirectory(thirdparty/glslang)
add_subdirectory(thirdparty/spirv-cross)
add_subdirectory(thirdparty/shaderc)
add_subdirectory(thirdparty/assimp)

set(BASE_SRC
        src/base/slot_map.h src/base/delegate.h src/base/event.h src/base/key_codes.h src/base/mouse_codes.h src/base/color.h src/base/color.cpp src/base/math.h src/base/math.cpp src/base/cursor.h src/base/iterator_range.h src/base/profiler.h src/base/profiler.cpp src/base/macro.h src/base/log.h src/base/log.cpp
        src/base/guid.cpp
        src/base/detector.h src/base/timer.cpp src/base/timer.h src/base/type_name.h src/base/memory.h src/base/allocator.cpp src/base/allocator.h src/base/flags.h src/base/crc32.h)

set(CORE_SRC
        src/core/ecs.h src/core/ecs.cpp
        src/core/world.h
        src/core/input_system.cpp src/core/input_system.h
        src/core/world.cpp src/core/meta/registration.h src/core/meta/type.h src/core/meta/type_info.h src/core/meta/type_info.cpp
        src/core/simulation.cpp src/core/simulation.h src/gfx/shader_repository.cpp src/gfx/shader_repository.h src/core/engine_events.cpp src/core/engine_events.h src/core/components/mesh_component.cpp src/core/components/mesh_component.h src/core/systems_registry.cpp src/core/systems_registry.h src/core/render_pipeline.cpp src/core/render_pipeline.h src/core/components/camera_component.cpp src/core/components/camera_component.h src/core/texture_compiler.cpp src/core/texture_compiler.h src/core/simulation_events.h src/core/component_loader.h src/core/viewer_registry.cpp src/core/viewer_registry.h src/core/viewer.h src/core/viewport.cpp src/core/viewport.h
        src/core/components/transform_component.cpp src/core/components/transform_component.h
        src/core/register_components.cpp src/core/register_components.h src/core/schema.cpp src/core/schema.h src/core/meta/interface_registry.cpp src/core/meta/interface_registry.h src/core/meta/interface.h
        src/core/asset_repository.cpp src/core/asset_repository.h
        src/core/components/version_component.h src/core/dcc_asset.cpp src/core/dcc_asset.h)

set(GFX_SRC
        src/gfx/gfx.h src/core/renderer.cpp src/core/renderer.h src/gfx/render_context.h src/gfx/command_buffers.cpp src/gfx/command_buffers.h src/gfx/render_context_opengl.cpp src/gfx/render_context_opengl.h src/gfx/shader.cpp src/gfx/shader.h src/gfx/vertex_layout_desc.cpp src/gfx/vertex_layout_desc.h src/gfx/shader_compiler.h src/gfx/shader_compiler_opengl.cpp src/gfx/shader_compiler_opengl.h src/core/assets_filesystem.cpp src/core/assets_filesystem.h src/core/texture.cpp src/core/texture.h)

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
        spirv-cross-glsl
        shaderc
        assimp
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

add_executable(
        host
        ${SANDBOX_DIR}/src/load_libraries.cpp
        ${SANDBOX_DIR}/src/sandbox.cpp ${SANDBOX_DIR}/src/sandbox.h
    )

target_link_libraries(host PRIVATE game ${ENGINE_LIBS})
target_include_directories(host PRIVATE ${ENGINE_INCLUDES})

# ----------------------------------------- #
# ------------ Sandbox plugins ------------ #
# ----------------------------------------- #

add_plugin(sandbox
        FILES
        ${SANDBOX_DIR}/src/sandbox.cpp ${SANDBOX_DIR}/src/sandbox.h
        LIBRARIES engine)


# ----------------------------------------- #
# -------------- Editor lib --------------- #
# ----------------------------------------- #

add_library(editor SHARED
        src/editor/gui.cpp
        src/editor/editor_tab.cpp
        src/editor/editor_tab_manager.cpp src/editor/editor_tab_manager.h)

target_link_libraries(editor PRIVATE engine)
target_link_libraries(editor PUBLIC imgui)

target_include_directories(editor PUBLIC src/editor/)

# ----------------------------------------- #
# ------------ Editor plugins ------------- #
# ----------------------------------------- #
add_plugin (editor_plugin
        FILES sandbox/src/editor_plugin.cpp
        LIBRARIES engine editor
    )

# ----------------------------------------- #
# ----------- Editor executable ----------- #
# ----------------------------------------- #

add_executable(ubik
        src/editor/editor.cpp
        src/editor/library_loader.cpp src/editor/library_loader.h src/editor/editor_tab_manager.cpp src/editor/editor_tab_manager.h)

target_link_libraries(ubik PRIVATE engine editor)

# TODO
list(APPEND UBIK_DEPENDENCIES sandbox)
add_dependencies(ubik ${UBIK_DEPENDENCIES})

# ----------------------------------------- #

add_executable(experimental
        src/gfx/experimental/experimental.cpp
        src/editor/library_loader.cpp
        )

target_link_libraries(experimental PRIVATE engine editor)
target_include_directories(experimental PRIVATE ${ENGINE_INCLUDES})



