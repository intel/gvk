
################################################################################
# gvk-string
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-string/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-string/")
gvk_add_static_library(
    target
        gvk-string
    folder
        "gvk-string/"
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/printer.hpp"
        "${includePath}/to-string.hpp"
        "${includePath}/utilities.hpp"
        "${includeDirectory}/gvk-string.hpp"
    sourceFiles
        "${sourcePath}/utilities.cpp"
)

################################################################################
# gvk-string.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    target
        gvk-string
    folder
        "gvk-string/"
    sourceFiles
        "${testsPath}/printer.tests.cpp"
        "${testsPath}/utilities.tests.cpp"
)
