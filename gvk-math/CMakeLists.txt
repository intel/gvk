
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
        glm-header-only
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

################################################################################
# gvk-math install
if(gvk-math_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET gvk-math)
endif()
if(gvk-math_INSTALL_HEADERS)
    gvk_install_headers(TARGET gvk-math)
endif()
