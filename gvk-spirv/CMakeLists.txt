
################################################################################
# gvk-spirv
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-spirv/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-spirv/")
gvk_add_static_library(
    TARGET
        gvk-spirv
    FOLDER
        "gvk-spirv/"
    LINK_LIBRARIES
        gvk-handles
        SPIRV-Headers
        SPIRV-Tools-static
        ${glslangLibraries}
        ${spirvCrossLibraries}
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
        "${includePath}/context.hpp"
        "${includeDirectory}/gvk-spirv.hpp"
    SOURCE_FILES
        "${sourcePath}/context.cpp"
)

################################################################################
# gvk-spirv.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    TARGET
        gvk-spirv
    FOLDER
        "gvk-spirv/"
    SOURCE_FILES
        "${testsPath}/context.tests.cpp"
)

################################################################################
# gvk-spirv install
gvk_install_library(TARGET gvk-spirv)
