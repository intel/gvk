
################################################################################
# gvk-spirv
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk/spirv/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk/spirv/")
gvk_add_static_library(
    target
        gvk-spirv
    linkLibraries
        gvk-core
        glslang_INTERFACE
        SPIRV-Cross_INTERFACE
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/context.hpp"
    sourceFiles
        "${sourcePath}/context.cpp"
)

################################################################################
# gvk-spirv.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    target
        gvk-spirv
    sourceFiles
        "${testsPath}/context.tests.cpp"
)
