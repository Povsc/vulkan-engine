cmake_minimum_required(VERSION 3.28)
project(vkrenderer LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(Shaders)

option(ENABLE_ASAN "Build with AddressSanitizer" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

find_package(Vulkan 1.4.335 REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(Stb REQUIRED)

add_executable(vkrenderer src/main.cpp)
target_compile_features(vkrenderer PRIVATE cxx_std_20)
target_compile_options(vkrenderer PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
target_compile_definitions(vkrenderer PRIVATE
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)
target_include_directories(vkrenderer PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(vkrenderer PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
    tinyobjloader::tinyobjloader
)

if(ENABLE_ASAN)
    target_compile_options(vkrenderer PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(vkrenderer PRIVATE -fsanitize=address)
endif()

file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.slang")
add_slang_shaders(vkrenderer
    SOURCES ${SHADER_SOURCES}
    OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders"
)