
################################################################################
# gvk-pipeline-explorer-plugin-factory
gvk_add_static_library(
    TARGET
        gvk-pipeline-explorer-plugin-factory
    FOLDER
        "gvk-pipeline-explorer/"
    LINK_LIBRARIES
        gvk-handles
        gvk-pipeline-explorer-info
        gvk-system
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
        "${includePath}/plugin-factory/basic-plugin.hpp"
        "${includePath}/plugin-factory/plugin-manager.hpp"
    SOURCE_FILES
        "${sourcePath}/plugin-factory/basic-plugin.cpp"
        "${sourcePath}/plugin-factory/plugin-manager.cpp"
)
