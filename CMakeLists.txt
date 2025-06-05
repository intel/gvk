
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
project(gvk)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install/" CACHE PATH "..." FORCE)
endif()
include(gvk.build)

################################################################################
# Vulkan SDK options
set(gvk-Vulkan-SDK_VERSION 1.4.313.0 CACHE STRING "")

set(gvk-Vulkan-SDK_URL_LINUX     "https://sdk.lunarg.com/sdk/download/${gvk-Vulkan-SDK_VERSION}/linux"   CACHE STRING "")
set(gvk-Vulkan-SDK_PACKAGE_LINUX "vulkansdk-linux-x86_64-${gvk-Vulkan-SDK_VERSION}.tar.xz"               CACHE STRING "")
set(gvk-Vulkan-SDK_SHA256_LINUX  4e957b66ade85eeaee95932aa7e3b45aea64db373c58a5eaefc8228cc71445c2        CACHE STRING "")

set(gvk-Vulkan-SDK_URL_WIN32     "https://sdk.lunarg.com/sdk/download/${gvk-Vulkan-SDK_VERSION}/windows" CACHE STRING "")
set(gvk-Vulkan-SDK_PACKAGE_WIN32 "vulkansdk-windows-X64-${gvk-Vulkan-SDK_VERSION}.exe"                   CACHE STRING "")
set(gvk-Vulkan-SDK_SHA256_WIN32  b643ca8ab4aea5c47b9c4e021a0b33b3a13871bf1d8131e162a9e48c257c4694        CACHE STRING "")

if(UNIX AND NOT APPLE)
    set(gvk-Vulkan-SDK_URL     "${gvk-Vulkan-SDK_URL_LINUX}"     CACHE STRING "")
    set(gvk-Vulkan-SDK_PACKAGE "${gvk-Vulkan-SDK_PACKAGE_LINUX}" CACHE STRING "")
    set(gvk-Vulkan-SDK_SHA256  ${gvk-Vulkan-SDK_SHA256_LINUX}    CACHE STRING "")
elseif(WIN32)
    set(gvk-Vulkan-SDK_URL     "${gvk-Vulkan-SDK_URL_WIN32}"     CACHE STRING "")
    set(gvk-Vulkan-SDK_PACKAGE "${gvk-Vulkan-SDK_PACKAGE_WIN32}" CACHE STRING "")
    set(gvk-Vulkan-SDK_SHA256  ${gvk-Vulkan-SDK_SHA256_WIN32}    CACHE STRING "")
endif()

################################################################################
# Default options
option(gvk-default_ENABLED           "" ON)
option(gvk-default_INSTALL_ARTIFACTS "" ON)
option(gvk-default_INSTALL_HEADERS   "" ON)

################################################################################
# External modules
macro(gvk_declare_external_module external-module enabled)
    option(${external-module}_ENABLED           "" ${enabled})
    option(${external-module}_INSTALL_ARTIFACTS "" ${gvk-default_INSTALL_ARTIFACTS})
    option(${external-module}_INSTALL_HEADERS   "" ${gvk-default_INSTALL_HEADERS})
    list(APPEND external-modules ${external-module})
endmacro()

function(gvk_enable_external_module external-module)
    set(${external-module}_ENABLED           ON                               CACHE BOOL "")
    set(${external-module}_INSTALL_ARTIFACTS ${gvk-default_INSTALL_ARTIFACTS} CACHE BOOL "")
    set(${external-module}_INSTALL_HEADERS   ${gvk-default_INSTALL_HEADERS}   CACHE BOOL "")
    include(external/${external-module})
endfunction()

gvk_declare_external_module(boost-asio            ${gvk-default_ENABLED})
gvk_declare_external_module(boost-multiprecision  ${gvk-default_ENABLED})
gvk_declare_external_module(cereal                ${gvk-default_ENABLED})
gvk_declare_external_module(glfw                  ${gvk-default_ENABLED})
gvk_declare_external_module(glm                   ${gvk-default_ENABLED})
gvk_declare_external_module(glslang               ${gvk-default_ENABLED})
gvk_declare_external_module(imgui                 ${gvk-default_ENABLED})
gvk_declare_external_module(SPIRV-Cross           ${gvk-default_ENABLED})
gvk_declare_external_module(SPIRV-Headers         ${gvk-default_ENABLED})
gvk_declare_external_module(SPIRV-Tools           ${gvk-default_ENABLED})
gvk_declare_external_module(stb                   ${gvk-default_ENABLED})
gvk_declare_external_module(tinyxml2              ${gvk-default_ENABLED})
gvk_declare_external_module(Vulkan                ${gvk-default_ENABLED})
gvk_declare_external_module(VulkanMemoryAllocator ${gvk-default_ENABLED})

foreach(external-module IN LISTS external-modules)
    if(${external-module}_ENABLED)
        gvk_enable_external_module(${external-module})
    endif()
endforeach()

################################################################################
# GVK modules
macro(gvk_decalre_module gvk-module enabled)
    option(${gvk-module}_ENABLED           "" ${enabled})
    option(${gvk-module}_INSTALL_ARTIFACTS "" ${gvk-default_INSTALL_ARTIFACTS})
    option(${gvk-module}_INSTALL_HEADERS   "" ${gvk-default_INSTALL_HEADERS})
    list(APPEND gvk-modules ${gvk-module})
endmacro()

function(gvk_enable_module gvk-module)
    set(${gvk-module}_ENABLED           ON                               CACHE BOOL   "")
    set(${gvk-module}_INSTALL_ARTIFACTS ${gvk-default_INSTALL_ARTIFACTS} CACHE BOOL   "")
    set(${gvk-module}_INSTALL_HEADERS   ${gvk-default_INSTALL_HEADERS}   CACHE BOOL   "")
    set(${gvk-module}_SOURCE_DIRECTORY  ${gvk-module}                    CACHE STRING "")
    if(EXISTS "${${gvk-module}_SOURCE_DIRECTORY}/dependencies.cmake")
        include("${${gvk-module}_SOURCE_DIRECTORY}/dependencies.cmake")
    endif()
endfunction()

set(gvk-ide-folder "" CACHE STRING "")
gvk_decalre_module(gvk-command-structures ${gvk-default_ENABLED})
gvk_decalre_module(gvk-containers         ${gvk-default_ENABLED})
gvk_decalre_module(gvk-cppgen             ${gvk-default_ENABLED})
gvk_decalre_module(gvk-format-info        ${gvk-default_ENABLED})
gvk_decalre_module(gvk-gui                ${gvk-default_ENABLED})
gvk_decalre_module(gvk-handles            ${gvk-default_ENABLED})
gvk_decalre_module(gvk-layer              ${gvk-default_ENABLED})
gvk_decalre_module(gvk-math               ${gvk-default_ENABLED})
gvk_decalre_module(gvk-pipeline-explorer  ${gvk-default_ENABLED})
gvk_decalre_module(gvk-reference          ${gvk-default_ENABLED})
gvk_decalre_module(gvk-restore-info       OFF)
gvk_decalre_module(gvk-restore-point      OFF)
gvk_decalre_module(gvk-runtime            ${gvk-default_ENABLED})
gvk_decalre_module(gvk-spirv              ${gvk-default_ENABLED})
gvk_decalre_module(gvk-state-tracker      OFF)
gvk_decalre_module(gvk-string             ${gvk-default_ENABLED})
gvk_decalre_module(gvk-structures         ${gvk-default_ENABLED})
gvk_decalre_module(gvk-system             ${gvk-default_ENABLED})
gvk_decalre_module(gvk-virtual-swapchain  OFF)
gvk_decalre_module(gvk-xml                ${gvk-default_ENABLED})

foreach(gvk-module IN LISTS gvk-modules)
    if(${gvk-module}_ENABLED)
        gvk_enable_module(${gvk-module})
    endif()
endforeach()

foreach(gvk-module IN LISTS gvk-modules)
    if(${gvk-module}_ENABLED)
        add_subdirectory(${${gvk-module}_SOURCE_DIRECTORY})
    endif()
endforeach()

################################################################################
# GVK samples and tests options
option(gvk-build-samples "" ${gvk-default_ENABLED})
option(gvk-build-tests   "" ${gvk-default_ENABLED})
option(gvk-run-tests     "" ${gvk-build-tests})

if(gvk-build-samples)
    add_subdirectory(samples)
endif()

if(gvk-build-tests)
    include(external/googletest)
endif()

################################################################################
# Install
gvk_install_package()
