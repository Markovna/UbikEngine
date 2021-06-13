cmake_minimum_required(VERSION 3.17)
project(engine)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(thirdparty/glfw glfw)
add_subdirectory(thirdparty/glad glad)
add_subdirectory(thirdparty/spdlog spdlog)
add_subdirectory(thirdparty/stb_image stb_image)
add_subdirectory(thirdparty/efsw efsw)

# ----------------------------------------- #
# ----------- Engine static lib ----------- #
# ----------------------------------------- #

set(BASE_SRC
        src/base/slot_map.h src/base/delegate.h src/base/event.h src/base/key_codes.h src/base/mouse_codes.h src/base/color.h src/base/color.cpp src/base/math.h src/base/math.cpp src/base/cursor.h src/base/iterator_range.h src/base/profiler.h src/base/profiler.cpp src/base/macro.h src/base/log.h src/base/log.cpp
        src/base/guid.cpp
        )

set(CORE_SRC
        src/core/ecs.h src/core/ecs.cpp
        src/core/plugins_registry.h src/core/plugins_registry.cpp src/core/world.h src/core/engine.cpp src/core/engine.h src/core/renderer.cpp src/core/renderer.h
        src/core/input_system.cpp src/core/input_system.h
        src/core/assets/assets.cpp src/core/assets/assets.h src/core/assets/texture.cpp src/core/assets/texture.h src/core/assets/shader.cpp src/core/assets/shader.h src/core/components/mesh_component.cpp src/core/components/mesh_component.h src/core/components/camera_component.cpp src/core/components/camera_component.h src/core/world.cpp src/core/meta/registration.h src/core/meta/registration.cpp src/core/components/component.cpp src/core/components/component.h src/core/serialization.h src/core/meta/type.h src/core/meta/type_info.h src/core/meta/type_info.cpp src/core/meta/schema.cpp src/core/meta/schema.h)

set(GFX_SRC
        src/gfx/gfx.cpp src/gfx/gfx.h src/gfx/gfx_config.h src/gfx/gfx_details.cpp src/gfx/gfx_details.h src/gfx/renderer_api.h src/gfx/renderer_api_gl.h src/gfx/renderer_api_gl.cpp
        )

set(PLATFORM_SRC
        src/platform/window.h src/platform/window.cpp src/base/window_event.h
        src/platform/os.cpp src/platform/os.h
        )

add_library(engine STATIC
        ${BASE_SRC} ${CORE_SRC} ${GFX_SRC} ${PLATFORM_SRC} src/core/game.cpp
        )

target_link_libraries(engine PRIVATE
        glfw
        glad
        spdlog
        stb_image)
target_include_directories(engine PRIVATE src)

set(ENGINE_LIBS
        engine
        glfw
        glad
        spdlog
        stb_image)

set(ENGINE_INCLUDES src)

# ----------------------------------------- #
# ----------- Engine shared lib ----------- #
# ----------------------------------------- #

add_library(engine_shared SHARED
        ${BASE_SRC} ${CORE_SRC} ${GFX_SRC} ${PLATFORM_SRC}
        )

target_link_libraries(engine_shared PUBLIC
        glfw
        glad
        spdlog
        stb_image)
target_include_directories(engine_shared PRIVATE src)

# ----------------------------------------- #
# ----------- Sandbox executable ---------- #
# ----------------------------------------- #

set(SANDBOX_DIR ${CMAKE_SOURCE_DIR}/sandbox/)

add_executable(game
        ${SANDBOX_DIR}/src/load_libraries.cpp
        ${SANDBOX_DIR}/src/spin_plugin.cpp ${SANDBOX_DIR}/src/spin_plugin.h
        ${SANDBOX_DIR}/src/sandbox_plugin.cpp ${SANDBOX_DIR}/src/sandbox_plugin.h)

target_link_libraries(game PRIVATE ${ENGINE_LIBS})
target_include_directories(game PRIVATE ${ENGINE_INCLUDES})

# ----------------------------------------- #
# -------------- Sandbox libs ------------- #
# ----------------------------------------- #

add_library(spin_plugin SHARED
        ${SANDBOX_DIR}/src/spin_plugin.cpp ${SANDBOX_DIR}/src/spin_plugin.h
        )
target_link_libraries(spin_plugin PRIVATE engine_shared)
target_include_directories(spin_plugin PRIVATE ${ENGINE_INCLUDES})

set_target_properties(spin_plugin
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${SANDBOX_DIR}/libs"
        LIBRARY_OUTPUT_DIRECTORY "${SANDBOX_DIR}/libs"
        RUNTIME_OUTPUT_DIRECTORY "${SANDBOX_DIR}/libs"
        )

add_library(sandbox_plugin SHARED
        ${SANDBOX_DIR}/src/spin_plugin.cpp ${SANDBOX_DIR}/src/spin_plugin.h
        ${SANDBOX_DIR}/src/sandbox_plugin.cpp ${SANDBOX_DIR}/src/sandbox_plugin.h
        )
target_link_libraries(sandbox_plugin PRIVATE engine_shared)
target_include_directories(sandbox_plugin PRIVATE ${ENGINE_INCLUDES})

set_target_properties(sandbox_plugin
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${SANDBOX_DIR}/libs"
        LIBRARY_OUTPUT_DIRECTORY "${SANDBOX_DIR}/libs"
        RUNTIME_OUTPUT_DIRECTORY "${SANDBOX_DIR}/libs"
        )

# ----------------------------------------- #
# ----------- Editor executable ----------- #
# ----------------------------------------- #

add_executable(editor src/editor/editor.cpp src/platform/file_watcher.h src/platform/file_watcher.cpp src/editor/library_registry.cpp src/editor/library_registry.h)

target_link_libraries(editor PRIVATE engine_shared efsw)
target_include_directories(editor PRIVATE ${ENGINE_INCLUDES})



