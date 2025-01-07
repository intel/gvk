
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
project(gvk)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
include(gvk.build)

################################################################################
# Default option
option(gvk-default_ENABLED           "" ON)
option(gvk-default_INSTALL_ARTIFACTS "" ON)
option(gvk-default_INSTALL_HEADERS   "" ON)

################################################################################
# Dependency options
option(gvk-asio_ENABLED                  "" ${gvk-default_ENABLED})
option(gvk-cereal_ENABLED                "" ${gvk-default_ENABLED})
option(gvk-glfw_ENABLED                  "" ${gvk-default_ENABLED})
option(gvk-glm_ENABLED                   "" ${gvk-default_ENABLED})
option(gvk-glslang_ENABLED               "" ${gvk-default_ENABLED})
option(gvk-imgui_ENABLED                 "" ${gvk-default_ENABLED})
option(gvk-SPIRV-Cross_ENABLED           "" ${gvk-default_ENABLED})
option(gvk-SPIRV-Headers_ENABLED         "" ${gvk-default_ENABLED})
option(gvk-SPIRV-Tools_ENABLED           "" ${gvk-default_ENABLED})
option(gvk-stb_ENABLED                   "" ${gvk-default_ENABLED})
option(gvk-tinyxml2_ENABLED              "" ${gvk-default_ENABLED})
option(gvk-Vulkan_ENABLED                "" ${gvk-default_ENABLED})
option(gvk-VulkanMemoryAllocator_ENABLED "" ${gvk-default_ENABLED})

################################################################################
# GVK options
set(gvk-ide-folder "" CACHE STRING "")
option(gvk-command-structures_ENABLED    "" OFF)
option(gvk-cppgen_ENABLED                "" ${gvk-default_ENABLED})
option(gvk-format-info_ENABLED           "" ${gvk-default_ENABLED})
option(gvk-gui_ENABLED                   "" ${gvk-default_ENABLED})
option(gvk-handles_ENABLED               "" ${gvk-default_ENABLED})
option(gvk-layer_ENABLED                 "" OFF)
option(gvk-math_ENABLED                  "" ${gvk-default_ENABLED})
option(gvk-reference_ENABLED             "" ${gvk-default_ENABLED})
option(gvk-restore-info_ENABLED          "" OFF)
option(gvk-restore-point_ENABLED         "" OFF)
option(gvk-runtime_ENABLED               "" ${gvk-default_ENABLED})
option(gvk-spirv_ENABLED                 "" ${gvk-default_ENABLED})
option(gvk-state-tracker_ENABLED         "" OFF)
option(gvk-string_ENABLED                "" ${gvk-default_ENABLED})
option(gvk-structures_ENABLED            "" ${gvk-default_ENABLED})
option(gvk-system_ENABLED                "" ${gvk-default_ENABLED})
option(gvk-virtual-swapchain_ENABLED     "" OFF)
option(gvk-xml_ENABLED                   "" ${gvk-default_ENABLED})
option(gvk-build-samples                 "" ${gvk-default_ENABLED})
option(gvk-build-tests                   "" ${gvk-default_ENABLED})
cmake_dependent_option(gvk-run-tests "Run tests as part of build and treat test failures as build failures" ON "gvk-build-tests" OFF)

################################################################################
# GVK include
function(gvk_include prefix name)
    if(${gvk-${name}_ENABLED})
        include(${prefix}/${name})
    endif()
endfunction()

################################################################################
# Include external dependencies
gvk_include(external asio)
gvk_include(external cereal)
gvk_include(external glfw)
gvk_include(external glm)
gvk_include(external glslang)
gvk_include(external imgui)
gvk_include(external SPIRV-Cross)
gvk_include(external SPIRV-Headers)
gvk_include(external SPIRV-Tools)
gvk_include(external stb)
gvk_include(external tinyxml2)
gvk_include(external Vulkan)
gvk_include(external VulkanMemoryAllocator)
if(gvk-build-tests)
    include(external/googletest)
endif()

################################################################################
# Include GVK libraries
gvk_include(gvk command-structures)
gvk_include(gvk cppgen)
gvk_include(gvk format-info)
gvk_include(gvk gui)
gvk_include(gvk handles)
gvk_include(gvk layer)
gvk_include(gvk math)
gvk_include(gvk reference)
gvk_include(gvk restore-info)
gvk_include(gvk restore-point)
gvk_include(gvk runtime)
gvk_include(gvk spirv)
gvk_include(gvk state-tracker)
gvk_include(gvk string)
gvk_include(gvk structures)
gvk_include(gvk system)
gvk_include(gvk virtual-swapchain)
gvk_include(gvk xml)

################################################################################
# Samples
if(gvk-build-samples)
    add_subdirectory(samples)
endif()

################################################################################
# Install
gvk_install_package()
