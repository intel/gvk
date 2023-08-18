
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
project(gvk)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
include(gvk.build)

################################################################################
# Options
include(CMakeDependentOption)
set                   (GVK_IDE_FOLDER               "" CACHE STRING "")
option                (GVK_ASIO_ENABLED             "" ON)
option                (GVK_CEREAL_ENABLED           "" ON)
option                (GVK_GLFW_ENABLED             "" ON)
option                (GVK_GLM_ENABLED              "" ON)
option                (GVK_GLSLANG_ENABLED          "" ON)
option                (GVK_IMGUI_ENABLED            "" ON)
option                (GVK_SPIRV_CROSS_ENABLED      "" ON)
option                (GVK_SPIRV_HEADERS_ENABLED    "" ON)
option                (GVK_SPIRV_TOOLS_ENABLED      "" ON)
option                (GVK_STB_ENABLED              "" ON)
option                (GVK_TINY_XML_ENABLED         "" ON)
option                (GVK_VMA_ENABLED              "" ON)
option                (GVK_BUILD_REFERENCE          "" ON)
option                (GVK_BUILD_STRING             "" ON)
cmake_dependent_option(GVK_BUILD_MATH               "" ON "GVK_GLM_ENABLED" OFF)
cmake_dependent_option(GVK_BUILD_XML                "" ON "GVK_BUILD_STRING;GVK_TINY_XML_ENABLED" OFF)
cmake_dependent_option(GVK_BUILD_CPPGEN             "" ON "GVK_BUILD_XML" OFF)
cmake_dependent_option(GVK_BUILD_RUNTIME            "" ON "GVK_BUILD_CPPGEN" OFF)
cmake_dependent_option(GVK_BUILD_STRUCTURES         "" ON "GVK_BUILD_RUNTIME;GVK_CEREAL_ENABLED" OFF)
cmake_dependent_option(GVK_BUILD_COMMAND_STRUCTURES "" ON "GVK_BUILD_STRUCTURES" OFF)
cmake_dependent_option(GVK_BUILD_FORMAT_INFO        "" ON "GVK_BUILD_STRUCTURES" OFF)
cmake_dependent_option(GVK_BUILD_HANDLES            "" ON "GVK_BUILD_FORMAT_INFO;GVK_BUILD_REFERENCE;GVK_VMA_ENABLED" OFF)
cmake_dependent_option(GVK_BUILD_LAYER              "" ON "GVK_BUILD_RUNTIME" OFF)
cmake_dependent_option(GVK_BUILD_SPIRV              "" ON "GVK_BUILD_HANDLES;GVK_GLSLANG_ENABLED;GVK_SPIRV_CROSS_ENABLED;GVK_SPIRV_HEADERS_ENABLED;GVK_SPIRV_TOOLS_ENABLED" OFF)
cmake_dependent_option(GVK_BUILD_STATE_TRACKER      "" ON "GVK_BUILD_COMMAND_STRUCTURES;GVK_BUILD_LAYER;GVK_BUILD_REFERENCE" OFF)
cmake_dependent_option(GVK_BUILD_RESTORE_POINT      "" ON "GVK_BUILD_STATE_TRACKER" OFF)
cmake_dependent_option(GVK_BUILD_SYSTEM             "" ON "GVK_BUILD_REFERENCE;GVK_GLFW_ENABLED" OFF)
cmake_dependent_option(GVK_BUILD_GUI                "" ON "GVK_BUILD_HANDLES;GVK_BUILD_SYSTEM;GVK_IMGUI_ENABLED" OFF)
cmake_dependent_option(GVK_BUILD_SAMPLES            "" ON "GVK_BUILD_GUI;GVK_BUILD_HANDLES;GVK_BUILD_MATH;GVK_BUILD_SPIRV;GVK_BUILD_SYSTEM;GVK_STB_ENABLED" OFF)
option                (GVK_BUILD_TESTS              "" ON)
cmake_dependent_option(GVK_RUN_TESTS                "" ON "GVK_BUILD_TESTS" OFF)
cmake_dependent_option(GVK_CREATE_TEST_PACKAGE      "" ON "GVK_BUILD_TESTS" OFF)
option                (GVK_CREATE_INSTALL_PACKAGE   "" OFF)

################################################################################
# Dependencies
find_package(Threads REQUIRED)
include(Vulkan)

macro(gvk_include_depenedency enabled dependency)
    if(${enabled})
        include(${dependency})
    endif()
endmacro()

# NOTE : Include SPIRV-Headers and SPIRV-Tools first so targets are available
#   when glslang and SPIRV-Cross are included
gvk_include_depenedency(GVK_SPIRV_HEADERS_ENABLED SPIRV-Headers)
gvk_include_depenedency(GVK_SPIRV_TOOLS_ENABLED   SPIRV-Tools)
gvk_include_depenedency(GVK_BUILD_TESTS           googletest)
gvk_include_depenedency(GVK_ASIO_ENABLED          asio)
gvk_include_depenedency(GVK_CEREAL_ENABLED        cereal)
gvk_include_depenedency(GVK_GLFW_ENABLED          glfw)
gvk_include_depenedency(GVK_GLM_ENABLED           glm)
gvk_include_depenedency(GVK_GLSLANG_ENABLED       glslang)
gvk_include_depenedency(GVK_SPIRV_CROSS_ENABLED   SPIRV-Cross)
gvk_include_depenedency(GVK_IMGUI_ENABLED         imgui)
gvk_include_depenedency(GVK_STB_ENABLED           stb)
gvk_include_depenedency(GVK_TINY_XML_ENABLED      tinyxml2)
gvk_include_depenedency(GVK_VMA_ENABLED           VulkanMemoryAllocator)

################################################################################
# Modules
macro(gvk_add_subdirectory enabled subdirectory)
    if(${enabled})
        add_subdirectory(${subdirectory})
    endif()
endmacro()

gvk_add_subdirectory(GVK_BUILD_COMMAND_STRUCTURES gvk-command-structures)
gvk_add_subdirectory(GVK_BUILD_CPPGEN             gvk-cppgen)
gvk_add_subdirectory(GVK_BUILD_LAYER              gvk-layer)
gvk_add_subdirectory(GVK_BUILD_RESTORE_POINT      gvk-restore-point)
gvk_add_subdirectory(GVK_BUILD_STATE_TRACKER      gvk-state-tracker)
gvk_add_subdirectory(GVK_BUILD_XML                gvk-xml)

add_library(gvk INTERFACE)
macro(gvk_add_interface enabled module)
    if(${enabled})
        add_subdirectory(${module})
        target_link_libraries(gvk INTERFACE ${module})
    endif()
endmacro()

gvk_add_interface(GVK_BUILD_RUNTIME     gvk-runtime)
gvk_add_interface(GVK_BUILD_HANDLES     gvk-handles)
gvk_add_interface(GVK_BUILD_FORMAT_INFO gvk-format-info)
gvk_add_interface(GVK_BUILD_GUI         gvk-gui)
gvk_add_interface(GVK_BUILD_MATH        gvk-math)
gvk_add_interface(GVK_BUILD_REFERENCE   gvk-reference)
gvk_add_interface(GVK_BUILD_SPIRV       gvk-spirv)
gvk_add_interface(GVK_BUILD_STRING      gvk-string)
gvk_add_interface(GVK_BUILD_STRUCTURES  gvk-structures)
gvk_add_interface(GVK_BUILD_SYSTEM      gvk-system)

################################################################################
# Samples
if(GVK_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

################################################################################
# Install
gvk_install_package()
