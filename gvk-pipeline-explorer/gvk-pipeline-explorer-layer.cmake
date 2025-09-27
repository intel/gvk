
################################################################################
# VK_LAYER_INTEL_gvk_pipeline_explorer
gvk_add_layer(
    TARGET
        VK_LAYER_INTEL_gvk_pipeline_explorer
    FOLDER
        "gvk-pipeline-explorer/"
    LINK_LIBRARIES
        gvk-pipeline-explorer-backend
    SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/VK_LAYER_INTEL_gvk_pipeline_explorer.cpp"
    DESCRIPTION
        "Intel(R) GPA Utilities for Vulkan* Pipeline Explorer"
)
