
################################################################################
# gvk-reference
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-reference/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-reference/")
gvk_add_static_library(
    target
        gvk-reference
    folder
        "gvk-reference/"
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/handle-id.hpp"
        "${includePath}/reference.hpp"
        "${includeDirectory}/gvk-reference.hpp"
    sourceFiles
        "${sourcePath}/reference.cpp"
)

################################################################################
# gvk-reference.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    target
        gvk-reference
    folder
        "gvk-reference/"
    linkLibraries
        asio_INTERFACE
    sourceFiles
        "${testsPath}/reference.tests.cpp"
)
