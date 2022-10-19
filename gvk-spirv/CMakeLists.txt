
################################################################################
# gvk-spirv
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk/spirv/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk/spirv/")
list(APPEND linkLibraries gvk-core)
if(GVK_GLSLANG_ENABLED)
    list(APPEND linkLibraries glslang_INTERFACE)
endif()
if(GVK_SPIRV_CROSS_ENABLED)
    list(APPEND linkLibraries SPIRV-Cross_INTERFACE)
endif()
gvk_add_static_library(
    target
        gvk-spirv
    linkLibraries
        ${linkLibraries}
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
