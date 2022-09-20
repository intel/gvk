
################################################################################
# gvk-math
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk/math/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk/math/")
set(linkLibraries "")
if(GVK_GLM_ENABLED)
    list(APPEND linkLibraries glm)
endif()
gvk_add_static_library(
    target
        gvk-math
    linkLibraries
        ${linkLibraries}
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/camera.hpp"
        "${includePath}/color.hpp"
        "${includePath}/defines.hpp"
        "${includePath}/transform.hpp"
    sourceFiles
        "${sourcePath}/camera.cpp"
        "${sourcePath}/transform.cpp"
)
