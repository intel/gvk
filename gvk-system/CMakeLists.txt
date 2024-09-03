
################################################################################
# gvk-system
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-system/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-system/")
gvk_add_static_library(
    TARGET
        gvk-system
    FOLDER
        "gvk-system/"
    LINK_LIBRARIES
        gvk-reference
        glfw
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
        "${includePath}/button-set.hpp"
        "${includePath}/input.hpp"
        "${includePath}/keyboard.hpp"
        "${includePath}/mouse.hpp"
        "${includePath}/surface.hpp"
        "${includePath}/time.hpp"
        "${includePath}/updateable.hpp"
        "${includeDirectory}/gvk-system.hpp"
    SOURCE_FILES
        "${sourcePath}/surface.cpp"
)

################################################################################
# gvk-system
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # TODO : Need to revisit cmake exports on Linux
    if(gvk-system_INSTALL_ARTIFACTS)
        gvk_install_artifacts(TARGET gvk-system)
    endif()
    if(gvk-system_INSTALL_HEADERS)
        gvk_install_headers(TARGET gvk-system)
    endif()
endif()
