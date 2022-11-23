
################################################################################
# gvk-reference
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk/")
gvk_add_static_library(
    target
        gvk-reference
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/reference.hpp"
    sourceFiles
        "${sourcePath}/reference.cpp"
)

################################################################################
# gvk-reference.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    target
        gvk-reference
    linkLibraries
        asio_INTERFACE
    sourceFiles
        "${testsPath}/reference.tests.cpp"
)
