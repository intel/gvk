
################################################################################
# gvk-containers
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-containers/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-containers/")
gvk_add_static_library(
    TARGET
        gvk-containers
    FOLDER
        "gvk-containers/"
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
        "${includePath}/detail/memory-usage-validator.hpp"
        "${includePath}/contiguous-set.hpp"
        "${includePath}/interval-tree.hpp"
        "${includePath}/thread-safe-unordered-map.hpp"
    SOURCE_FILES
        "${sourcePath}/detail/memory-usage-validator.cpp"
)

################################################################################
# gvk-containers.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    TARGET
        gvk-containers
    FOLDER
        "gvk-containers/"
    SOURCE_FILES
        "${testsPath}/interval-tree.tests.cpp"
)

################################################################################
# gvk-containers install
if(gvk-containers_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET gvk-containers)
endif()
if(gvk-containers_INSTALL_HEADERS)
    gvk_install_headers(TARGET gvk-containers)
endif()
