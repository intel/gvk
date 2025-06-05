
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
        "${includePath}/gpu-address-map.hpp"
        "${includePath}/gpu-memcpy.hpp"
        "${includeDirectory}/gvk-spirv.hpp"
    SOURCE_FILES
        "${sourcePath}/context.cpp"
        "${sourcePath}/gpu-address-map.cpp"
        "${sourcePath}/gpu-memcpy.cpp"
)

################################################################################
# gvk-spirv.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    TARGET
        gvk-spirv
    FOLDER
        "gvk-spirv/"
    LINK_LIBRARIES
        boost-multiprecision
    SOURCE_FILES
        "${testsPath}/context.tests.cpp"
        "${testsPath}/gpu-address-map.tests.cpp"
        "${testsPath}/gpu-memcpy.tests.cpp"
        "${testsPath}/raytracing.tests.cpp"
    INCLUDE_FILES
        "${testsPath}/spirv-validation-context.hpp"
        "${testsPath}/spirv-validation-utilities.hpp"
)

################################################################################
# gvk-spirv install
if(gvk-spirv_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET gvk-spirv)
endif()
if(gvk-spirv_INSTALL_HEADERS)
    gvk_install_headers(TARGET gvk-spirv)
endif()
