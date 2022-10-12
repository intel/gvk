
################################################################################
# gvk-system
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk/system/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk/system/")
set(linkLibraries "")
if(GVK_GLFW_ENABLED)
    list(APPEND linkLibraries glfw)
endif()
gvk_add_static_library(
    target
        gvk-system
    linkLibraries
        ${linkLibraries}
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/button-set.hpp"
        "${includePath}/input.hpp"
        "${includePath}/keyboard.hpp"
        "${includePath}/mouse.hpp"
        "${includePath}/random.hpp"
        "${includePath}/surface.hpp"
        "${includePath}/time.hpp"
        "${includePath}/updateable.hpp"
    sourceFiles
        "${sourcePath}/surface.cpp"
)

################################################################################
# gvk-system.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    target
        gvk-system
    sourceFiles
        "${testsPath}/random.tests.cpp"
)
