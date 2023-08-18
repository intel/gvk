
################################################################################
# gvk-reference
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-reference/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-reference/")
gvk_add_static_library(
    TARGET
        gvk-reference
    FOLDER
        "gvk-reference/"
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
        "${includePath}/handle-id.hpp"
        "${includePath}/reference.hpp"
        "${includeDirectory}/gvk-reference.hpp"
    SOURCE_FILES
        "${sourcePath}/reference.cpp"
)

################################################################################
# gvk-reference.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    TARGET
        gvk-reference
    FOLDER
        "gvk-reference/"
    LINK_LIBRARIES
        asio
    SOURCE_FILES
        "${testsPath}/reference.tests.cpp"
)

################################################################################
# gvk-reference install
gvk_install_library(TARGET gvk-reference)
