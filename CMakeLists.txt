# tsundoku — Vulkan Renderer

cmake_minimum_required(VERSION 3.20)
project(tsundoku CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

find_path(GLM_INCLUDE_DIR
    NAMES glm/glm.hpp
    PATHS /usr/include /usr/local/include
)

include(FetchContent)

FetchContent_Declare(cgltf
    GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
    GIT_TAG        master
)
FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(cgltf stb)

# Static library

set(LIB_SOURCES
    src/core/platform.cpp
    src/core/entity.cpp
    src/core/scene.cpp

    src/vulkan/vk_context.cpp
    src/vulkan/swapchain.cpp
    src/vulkan/render_pass.cpp
    src/vulkan/pipeline.cpp
    src/vulkan/framebuffer.cpp
    src/vulkan/commandbuffer.cpp
    src/vulkan/sync.cpp
    src/vulkan/frame_cap.cpp
    src/vulkan/vertex_buffer.cpp
    src/vulkan/descriptor_set.cpp
    src/vulkan/render_system.cpp

    src/renderer/camera_vulkan.cpp

    src/components/camera.cpp
)

add_library(tsundoku_lib STATIC ${LIB_SOURCES})

target_compile_options(tsundoku_lib PUBLIC
    -fno-omit-frame-pointer
    -g
)

target_include_directories(tsundoku_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${cgltf_SOURCE_DIR}
    ${stb_SOURCE_DIR}
    ${Vulkan_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
)

target_link_libraries(tsundoku_lib PUBLIC
    glfw
    Vulkan::Vulkan
    glm::glm
    ${CMAKE_DL_LIBS}
)

# Executable

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE tsundoku_lib)

# Shaders

set(SPIRV_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")

target_compile_definitions(tsundoku_lib PUBLIC
    SHADER_PATH="${SPIRV_OUTPUT_DIR}/"
)

find_program(GLSLC glslc)
if(NOT GLSLC)
    message(FATAL_ERROR "glslc not found")
endif()

file(GLOB_RECURSE GLSL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/shaders/*.glsl"
)

file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})

foreach(GLSL ${GLSL_SOURCES})
    get_filename_component(FILE_NAME ${GLSL} NAME_WE)
    set(SPIRV "${SPIRV_OUTPUT_DIR}/${FILE_NAME}.spv")

    if(FILE_NAME STREQUAL "vert")
        set(STAGE vertex)
    elseif(FILE_NAME STREQUAL "frag")
        set(STAGE fragment)
    elseif(FILE_NAME STREQUAL "comp")
        set(STAGE compute)
    else()
        message(WARNING "Unknown shader stage for: ${FILE_NAME}.glsl — skipping")
        continue()
    endif()

    add_custom_command(
        OUTPUT  ${SPIRV}
        COMMAND ${GLSLC} -fshader-stage=${STAGE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
        COMMENT "Compiling shader: ${FILE_NAME}.glsl"
        USES_TERMINAL
    )
    list(APPEND SPIRV_OUTPUTS ${SPIRV})
endforeach()

add_custom_target(tsundoku_shaders ALL DEPENDS ${SPIRV_OUTPUTS})
add_dependencies(${PROJECT_NAME} tsundoku_shaders)
