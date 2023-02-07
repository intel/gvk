
################################################################################
# gvk-system
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-system/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-system/")
set(linkLibraries "")
gvk_add_static_library(
    target
        gvk-system
    folder
        "gvk-system/"
    linkLibraries
        gvk-reference
        glfw
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/button-set.hpp"
        "${includePath}/input.hpp"
        "${includePath}/keyboard.hpp"
        "${includePath}/mouse.hpp"
        "${includePath}/surface.hpp"
        "${includePath}/time.hpp"
        "${includePath}/updateable.hpp"
        "${includeDirectory}/gvk-system.hpp"
    sourceFiles
        "${sourcePath}/surface.cpp"
)
