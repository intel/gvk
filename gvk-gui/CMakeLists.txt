
################################################################################
# gvk-gui
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-gui/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-gui/")
gvk_add_static_library(
    TARGET
        gvk-gui
    FOLDER
        "gvk-gui/"
    LINK_LIBRARIES
        gvk-handles
        gvk-spirv
        gvk-system
        imgui
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
        "${includePath}/renderer.hpp"
        "${includeDirectory}/gvk-gui.hpp"
    SOURCE_FILES
        "${sourcePath}/renderer.cpp"
)
