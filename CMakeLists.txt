
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
project(gvk)

################################################################################
# GVK build options
include(CMakeDependentOption)
# Configuration options
option                (GVK_NO_PROTOTYPES          "" OFF)
# Test options
option                (GVK_TESTS_ENABLED          "" ON)
cmake_dependent_option(GVK_RUN_TESTS              "" ON
    "GVK_TESTS_ENABLED" OFF
)
# Dependency options
option                (GVK_ASIO_ENABLED           "" ON)
option                (GVK_CEREAL_ENABLED         "" ON)
option                (GVK_GLFW_ENABLED           "" ON)
option                (GVK_GLM_ENABLED            "" ON)
option                (GVK_GLSLANG_ENABLED        "" ON)
option                (GVK_SPIRV_CROSS_ENABLED    "" ON)
option                (GVK_STB_ENABLED            "" ON)
option                (GVK_TINY_XML_ENABLED       "" ON)
option                (GVK_VMA_ENABLED            "" ON)
# Target options
option                (GVK_STRING_ENABLED         "" ON)
cmake_dependent_option(GVK_XML_ENABLED            "" ON
    "GVK_STRING_ENABLED;GVK_TINY_XML_ENABLED" OFF
)
option                (GVK_MATH_ENABLED           "" ON)
option                (GVK_SYSTEM_ENABLED         "" ON)
cmake_dependent_option(GVK_CORE_ENABLED           "" ON
    "GVK_STRING_ENABLED;GVK_XML_ENABLED;GVK_MATH_ENABLED;GVK_SYSTEM_ENABLED;GVK_ASIO_ENABLED;GVK_CEREAL_ENABLED;GVK_VMA_ENABLED" OFF
)
# Samples options
cmake_dependent_option(GVK_SAMPLES_ENABLED        "" ON
    "GVK_CORE_ENABLED;GVK_GLFW_ENABLED;GVK_GLM_ENABLED;GVK_GLSLANG_ENABLED;GVK_SPIRV_CROSS_ENABLED;GVK_STB_ENABLED" OFF
)
# CI options
option                (GVK_CREATE_CI_SCAN_PACKAGE "" OFF)
cmake_dependent_option(GVK_CREATE_CI_TEST_PACKAGE "" OFF
    "GVK_TESTS_ENABLED" OFF
)

################################################################################
# GVK CMake utilities
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/gvk.build.cmake")

################################################################################
# External dependencies
find_package(Vulkan REQUIRED)
set(Vulkan_XML "$ENV{VULKAN_SDK}/share/vulkan/registry/vk.xml")

if(GVK_TESTS_ENABLED)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/googletest.cmake")
endif()
if(GVK_ASIO_ENABLED)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/asio.cmake")
endif()
if(GVK_CEREAL_ENABLED)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cereal.cmake")
endif()
if(GVK_GLFW_ENABLED)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/glfw.cmake")
endif()
if(GVK_GLM_ENABLED)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/glm.cmake")
endif()
if(GVK_GLSLANG_ENABLED)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/glslang.cmake")
endif()
if(GVK_SPIRV_CROSS_ENABLED)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/SPIRV-Cross.cmake")
endif()
if(GVK_STB_ENABLED)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/stb.cmake")
endif()
if(GVK_TINY_XML_ENABLED)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/tinyxml2.cmake")
endif()
if(GVK_VMA_ENABLED)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/VulkanMemoryAllocator.cmake")
endif()

################################################################################
# GVK targets
if(GVK_STRING_ENABLED)
    add_subdirectory(gvk-string)
endif()
if(GVK_XML_ENABLED)
    add_subdirectory(gvk-xml)
endif()
if(GVK_MATH_ENABLED)
    add_subdirectory(gvk-math)
endif()
if(GVK_SYSTEM_ENABLED)
    add_subdirectory(gvk-system)
endif()
if(GVK_CORE_ENABLED)
    add_subdirectory(gvk)
endif()
if(GVK_SAMPLES_ENABLED AND NOT GVK_NO_PROTOTYPES)
    add_subdirectory(samples)
endif()
