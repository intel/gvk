
include_guard()

include(FetchContent)

set(SPIRV_CROSS_CLI                      OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS             OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON  CACHE BOOL "" FORCE)
set(SPIRV_CROSS_FORCE_PIC                ON  CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL             ON  CACHE BOOL "" FORCE)
set(SPIRV-Cross_VERSION 2de1265fca722929785d9acdec4ab728c47a0254) # sdk-1.3.268.0
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
    gvk_install_artifacts(TARGET ${spirvCrossTarget} VERSION ${SPIRV-Cross_VERSION})
endmacro()

gvk_setup_spirv_cross_target(spirv-cross-c)
gvk_setup_spirv_cross_target(spirv-cross-core)
gvk_setup_spirv_cross_target(spirv-cross-cpp)
gvk_setup_spirv_cross_target(spirv-cross-glsl)
gvk_setup_spirv_cross_target(spirv-cross-hlsl)
gvk_setup_spirv_cross_target(spirv-cross-msl)
gvk_setup_spirv_cross_target(spirv-cross-reflect)
gvk_setup_spirv_cross_target(spirv-cross-util)

if(GVK_CREATE_INSTALL_PACKAGE)
    file(GLOB spirvCrossHeaderFiles "${SPIRV-Cross_SOURCE_DIR}/*.h" "${SPIRV-Cross_SOURCE_DIR}/*.hpp")
    install(FILES ${spirvCrossHeaderFiles} DESTINATION include/spirv_cross/)
endif()
