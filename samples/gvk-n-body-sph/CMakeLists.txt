################################################################################
# gvk-n-body-sph
################################################################################

set(includeFiles
    "${CMAKE_CURRENT_LIST_DIR}/defines.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/utilities.hpp"
)

set(sourceFiles
    "${CMAKE_CURRENT_LIST_DIR}/utilities.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

gvk_add_executable(
    TARGET
        gvk-n-body-sph
    FOLDER
        "samples/gvk-n-body-sph"
    LINK_LIBRARIES
        gvk-command-structures
        gvk-format-info
        gvk-gui
        gvk-handles
        gvk-math
        gvk-spirv
        gvk-string
        gvk-structures
        gvk-system
    INCLUDE_FILES
        ${includeFiles}
    SOURCE_FILES
        ${sourceFiles}
)

if(VK_NO_PROTOTYPES)
    target_compile_definitions(gvk-n-body-sph PUBLIC VK_NO_PROTOTYPES=1)
endif()
