
################################################################################
# gvk-string
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-string/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-string/")
gvk_add_static_library(
    TARGET
        gvk-string
    FOLDER
        "gvk-string/"
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
        "${includePath}/printer.hpp"
        "${includePath}/to-string.hpp"
        "${includePath}/utilities.hpp"
        "${includeDirectory}/gvk-string.hpp"
    SOURCE_FILES
        "${sourcePath}/utilities.cpp"
)

################################################################################
# gvk-string.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    TARGET
        gvk-string
    FOLDER
        "gvk-string/"
    SOURCE_FILES
        "${testsPath}/printer.tests.cpp"
        "${testsPath}/utilities.tests.cpp"
)

################################################################################
# gvk-string install
if(gvk-string_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET gvk-string)
endif()
if(gvk-string_INSTALL_HEADERS)
    gvk_install_headers(TARGET gvk-string)
endif()
