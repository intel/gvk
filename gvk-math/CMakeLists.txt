
################################################################################
# gvk-math
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-math/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-math/")
gvk_add_static_library(
    target
        gvk-math
    folder
        "gvk-math/"
    linkLibraries
        glm
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/camera.hpp"
        "${includePath}/color.hpp"
        "${includePath}/defines.hpp"
        "${includePath}/transform.hpp"
        "${includeDirectory}/gvk-math.hpp"
    sourceFiles
        "${sourcePath}/camera.cpp"
        "${sourcePath}/transform.cpp"
)
