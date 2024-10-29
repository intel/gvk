
include_guard(GLOBAL)
gvk_enable_target(SPIRV-Cross)

set(SPIRV_CROSS_CLI          OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_FORCE_PIC    ON  CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL ON  CACHE BOOL "" FORCE)
set(SPIRV-Cross_VERSION 65d7393430f6c7bb0c20b6d53250fe04847cc2ae) # vulkan-sdk-1.3.296.0
FetchContent_Declare(
    SPIRV-Cross
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Cross.git"
    GIT_TAG ${SPIRV-Cross_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(SPIRV-Cross)
FetchContent_GetProperties(SPIRV-Cross SOURCE_DIR SPIRV-Cross_SOURCE_DIR)

macro(gvk_setup_spirv_cross_target spirvCrossTarget)
    list(APPEND spirvCrossLibraries ${spirvCrossTarget})
    set_target_properties(${spirvCrossTarget} PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/SPIRV-Cross/")
    if(gvk-SPIRV-Cross_INSTALL_ARTIFACTS)
        gvk_install_artifacts(TARGET ${spirvCrossTarget} VERSION ${SPIRV-Cross_VERSION})
    endif()
endmacro()

gvk_setup_spirv_cross_target(spirv-cross-c)
gvk_setup_spirv_cross_target(spirv-cross-core)
gvk_setup_spirv_cross_target(spirv-cross-cpp)
gvk_setup_spirv_cross_target(spirv-cross-glsl)
gvk_setup_spirv_cross_target(spirv-cross-hlsl)
gvk_setup_spirv_cross_target(spirv-cross-msl)
gvk_setup_spirv_cross_target(spirv-cross-reflect)
gvk_setup_spirv_cross_target(spirv-cross-util)
set(spirvCrossLibraries ${spirvCrossLibraries} PARENT_SCOPE)

if(gvk-SPIRV-Cross_INSTALL_HEADERS)
    file(GLOB spirvCrossHeaderFiles "${SPIRV-Cross_SOURCE_DIR}/*.h" "${SPIRV-Cross_SOURCE_DIR}/*.hpp")
    install(FILES ${spirvCrossHeaderFiles} DESTINATION include/spirv_cross/)
endif()
