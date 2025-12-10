
################################################################################
# VK_LAYER_INTEL_gvk_unique_handles
gvk_add_layer(
    TARGET
        VK_LAYER_INTEL_gvk_unique_handles
    FOLDER
        "VK_LAYER_INTEL_gvk_unique_handles/"
    LINK_LIBRARIES
        gvk-layer
    SOURCE_FILES
        "VK_LAYER_INTEL_gvk_unique_handles.cpp"
    DESCRIPTION
        "Intel(R) GPA Utilities for Vulkan* unique handles"
)

################################################################################
# VK_LAYER_INTEL_gvk_unique_handles install
if(VK_LAYER_INTEL_gvk_unique_handles_INSTALL_ARTIFACTS)
    gvk_install_layer(TARGET VK_LAYER_INTEL_gvk_unique_handles)
endif()
