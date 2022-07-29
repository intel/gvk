
set(SPIRV_CROSS_CLI          OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_FORCE_PIC    ON  CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL ON  CACHE BOOL "" FORCE)
FetchContent_Declare(
    SPIRV-Cross
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Cross.git"
    GIT_TAG 9acb9ec31f5a8ef80ea6b994bb77be787b08d3d1 # 2021-01-15
    GIT_PROGRESS TRUE
    FETCHCONTENT_UPDATES_DISCONNECTED
)
FetchContent_MakeAvailable(SPIRV-Cross)
set(folder "gvk/external/spirv-cross/")
set_target_properties(spirv-cross-c       PROPERTIES FOLDER "${folder}")
set_target_properties(spirv-cross-core    PROPERTIES FOLDER "${folder}")
set_target_properties(spirv-cross-cpp     PROPERTIES FOLDER "${folder}")
set_target_properties(spirv-cross-glsl    PROPERTIES FOLDER "${folder}")
set_target_properties(spirv-cross-hlsl    PROPERTIES FOLDER "${folder}")
set_target_properties(spirv-cross-msl     PROPERTIES FOLDER "${folder}")
set_target_properties(spirv-cross-reflect PROPERTIES FOLDER "${folder}")
set_target_properties(spirv-cross-util    PROPERTIES FOLDER "${folder}")
add_library(SPIRV-Cross_INTERFACE INTERFACE)
target_link_libraries(
    SPIRV-Cross_INTERFACE
    INTERFACE
        spirv-cross-c
        spirv-cross-core
        spirv-cross-cpp
        spirv-cross-glsl
        spirv-cross-hlsl
        spirv-cross-msl
        spirv-cross-reflect
        spirv-cross-util
)
