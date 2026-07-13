
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
        gvk-string
        gvk-system
        imgui
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
        "${includePath}/renderer.hpp"
        "${includePath}/utilities.hpp"
        "${includePath}/window.hpp"
        "${includePath}/window-manager.hpp"
        "${includeDirectory}/gvk-gui.hpp"
    SOURCE_FILES
        "${sourcePath}/renderer.cpp"
        "${sourcePath}/utilities.cpp"
)

################################################################################
# gvk-gui install
if(gvk-gui_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET gvk-gui)
endif()
if(gvk-gui_INSTALL_HEADERS)
    gvk_install_headers(TARGET gvk-gui)
endif()
