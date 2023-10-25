
################################################################################
# gvk-math
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-math/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-math/")
gvk_add_static_library(
    TARGET
        gvk-math
    FOLDER
        "gvk-math/"
    LINK_LIBRARIES
        glm
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
        "${includePath}/camera.hpp"
        "${includePath}/color.hpp"
        "${includePath}/defines.hpp"
        "${includePath}/transform.hpp"
        "${includeDirectory}/gvk-math.hpp"
    SOURCE_FILES
        "${sourcePath}/camera.cpp"
        "${sourcePath}/transform.cpp"
)
