
################################################################################
# VK_LAYER_INTEL_gvk_virtual_swapchain
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-virtual-swapchain/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-virtual-swapchain/")
gvk_add_layer(
    TARGET
        VK_LAYER_INTEL_gvk_virtual_swapchain
    FOLDER
        "VK_LAYER_INTEL_gvk_virtual_swapchain/"
    LINK_LIBRARIES
        gvk-handles
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
        "${includePath}/layer.hpp"
    SOURCE_FILES
        "${sourcePath}/layer.cpp"
    DESCRIPTION
        "Intel(R) GPA Utilities for Vulkan* virtual swapchain"
)

################################################################################
# VK_LAYER_INTEL_gvk_virtual_swapchain install
gvk_install_layer(TARGET VK_LAYER_INTEL_gvk_virtual_swapchain)
