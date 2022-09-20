
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 20)
project(gvk)

find_package(Vulkan REQUIRED)
set(Vulkan_SDK_DIR "${Vulkan_INCLUDE_DIRS}/../")
set(Vulkan_XML "${Vulkan_SDK_DIR}/share/vulkan/registry/vk.xml")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
# GVK CMake functions and utilities
include(gvk)
# External dependencies
include(asio)
include(cereal)
include(glfw)
include(glm)
include(glslang)
include(gtest)
include(SPIRV-Cross)
include(stb)
include(tinyxml2)
include(VulkanMemoryAllocator)

add_subdirectory(gvk)
add_subdirectory(gvk-math)
add_subdirectory(gvk-string)
add_subdirectory(gvk-system)
add_subdirectory(gvk-xml)
add_subdirectory(samples)
