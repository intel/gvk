
################################################################################
# gvk-string
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk/")
gvk_add_static_library(
    target
        gvk-string
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/printer.hpp"
        "${includePath}/string.hpp"
    sourceFiles
        "${sourcePath}/string.cpp"
)

################################################################################
# gvk-string.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    target
        gvk-string
    sourceFiles
        "${testsPath}/printer.tests.cpp"
        "${testsPath}/string.tests.cpp"
)
