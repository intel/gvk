
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
project(gvk)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
include(gvk.build)

################################################################################
# Options
include(CMakeDependentOption)
set                   (GVK_IDE_FOLDER          "" CACHE STRING "")
option                (GVK_BUILD_TESTS         "" ON)
cmake_dependent_option(GVK_RUN_TESTS           "" ON  "GVK_BUILD_TESTS" OFF)
cmake_dependent_option(GVK_CREATE_TEST_PACKAGE "" OFF "GVK_BUILD_TESTS" OFF)
option                (GVK_ASIO_ENABLED        "" ON)
option                (GVK_CEREAL_ENABLED      "" ON)
option                (GVK_GLFW_ENABLED        "" ON)
option                (GVK_GLM_ENABLED         "" ON)
option                (GVK_GLSLANG_ENABLED     "" ON)
option                (GVK_SPIRV_CROSS_ENABLED "" ON)
option                (GVK_STB_ENABLED         "" ON)
option                (GVK_TINY_XML_ENABLED    "" ON)
option                (GVK_VMA_ENABLED         "" ON)
option                (GVK_MATH_ENABLED        "" ON)
option                (GVK_SYSTEM_ENABLED      "" ON)
option                (GVK_STRING_ENABLED      "" ON)
cmake_dependent_option(GVK_XML_ENABLED         "" ON  "GVK_STRING_ENABLED;GVK_TINY_XML_ENABLED" OFF)
cmake_dependent_option(GVK_CPPGEN_ENABLED      "" ON  "GVK_STRING_ENABLED;GVK_XML_ENABLED" OFF)
cmake_dependent_option(GVK_CORE_ENABLED        "" ON  "GVK_CPPGEN_ENABLED;GVK_SYSTEM_ENABLED;GVK_ASIO_ENABLED;GVK_CEREAL_ENABLED;GVK_VMA_ENABLED" OFF)
cmake_dependent_option(GVK_SPIRV_ENABLED       "" ON  "GVK_CORE_ENABLED;GVK_GLSLANG_ENABLED;GVK_SPIRV_CROSS_ENABLED" OFF)
cmake_dependent_option(GVK_BUILD_SAMPLES       "" ON  "GVK_CORE_ENABLED;GVK_SPIRV_ENABLED;GVK_GLFW_ENABLED;GVK_GLM_ENABLED;GVK_STB_ENABLED" OFF)
option                (GVK_NO_PROTOTYPES       "" OFF)

################################################################################
# Dependencies
find_package(Vulkan REQUIRED)
if(MSVC)
    set(Vulkan_SDK_DIR "${Vulkan_INCLUDE_DIRS}/../")
    set(Vulkan_XML "${Vulkan_SDK_DIR}/share/vulkan/registry/vk.xml")
else()
    set(Vulkan_XML "$ENV{VULKAN_SDK}/share/vulkan/registry/vk.xml")
endif()

macro(gvk_include_depenedency enabled dependency)
    if(${enabled})
        include(${dependency})
    endif()
endmacro()

gvk_include_depenedency(${GVK_BUILD_TESTS}      googletest)
gvk_include_depenedency(${GVK_ASIO_ENABLED}     asio)
gvk_include_depenedency(${GVK_CEREAL_ENABLED}   cereal)
gvk_include_depenedency(${GVK_GLFW_ENABLED}     glfw)
gvk_include_depenedency(${GVK_GLM_ENABLED}      glm)
gvk_include_depenedency(${GVK_GLSLANG_ENABLED}  glslang)
gvk_include_depenedency(${GVK_GLSLANG_ENABLED}  SPIRV-Cross)
gvk_include_depenedency(${GVK_STB_ENABLED}      stb)
gvk_include_depenedency(${GVK_TINY_XML_ENABLED} tinyxml2)
gvk_include_depenedency(${GVK_VMA_ENABLED}      VulkanMemoryAllocator)

################################################################################
# Modules
add_library(gvk INTERFACE)
macro(gvk_add_module enabled module)
    if(${enabled})
        add_subdirectory(${module})
        target_link_libraries(gvk INTERFACE ${module})
    endif()
endmacro()

gvk_add_module(${GVK_CORE_ENABLED}   gvk-core)
gvk_add_module(${GVK_CPPGEN_ENABLED} gvk-cppgen)
gvk_add_module(${GVK_MATH_ENABLED}   gvk-math)
gvk_add_module(${GVK_SPIRV_ENABLED}  gvk-spirv)
gvk_add_module(${GVK_STRING_ENABLED} gvk-string)
gvk_add_module(${GVK_SYSTEM_ENABLED} gvk-system)
gvk_add_module(${GVK_XML_ENABLED}    gvk-xml)

################################################################################
# Samples
if(GVK_BUILD_SAMPLES AND NOT GVK_NO_PROTOTYPES)
    add_subdirectory(samples)
endif()
