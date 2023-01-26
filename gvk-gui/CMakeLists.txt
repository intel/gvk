
################################################################################
# gvk-gui
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-gui/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-gui/")
gvk_add_static_library(
    target
        gvk-gui
    folder
        "gvk-gui/"
    linkLibraries
        gvk-handles
        gvk-spirv
        gvk-system
        imgui
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/renderer.hpp"
        "${includeDirectory}/gvk-gui.hpp"
    sourceFiles
        "${sourcePath}/renderer.cpp"
)
