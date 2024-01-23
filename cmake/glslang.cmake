
include_guard()

include(FetchContent)

set(BUILD_TESTING           OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_INSTALL  OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_JS       OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_WEBMIN   OFF CACHE BOOL "" FORCE)
set(SKIP_GLSLANG_INSTALL    ON  CACHE BOOL "" FORCE)
set(glslang_VERSION a91631b260cba3f22858d6c6827511e636c2458a) # vulkan-sdk-1.3.275.0
FetchContent_Declare(
    glslang
    GIT_REPOSITORY "https://github.com/KhronosGroup/glslang.git"
    GIT_TAG ${glslang_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glslang)

macro(gvk_setup_glslang_target glslangTarget)
    list(APPEND glslangLibraries ${glslangTarget})
    set_target_properties(${glslangTarget} PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/glslang/")
    gvk_install_artifacts(TARGET ${glslangTarget} VERSION ${glslang_VERSION})
endmacro()

# HACK : glslang headers aren't being installed, but the export expects them to
#   be at `include/External`...this modifies the INSTALL_INTERFACE to `include`
#   to avoid an error for missing INTERFACE_INCLUDE_DIRECTORIES on import.
get_target_property(SPIRV_INTERFACE_INCLUDE_DIRECTORIES SPIRV INTERFACE_INCLUDE_DIRECTORIES)
string(REPLACE "$<INSTALL_INTERFACE:include/External>" "$<INSTALL_INTERFACE:include>" SPIRV_INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_INTERFACE_INCLUDE_DIRECTORIES}")
set_target_properties(SPIRV PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_INTERFACE_INCLUDE_DIRECTORIES}")

gvk_setup_glslang_target(GenericCodeGen)
gvk_setup_glslang_target(glslang)
gvk_setup_glslang_target(glslang-default-resource-limits)
gvk_setup_glslang_target(MachineIndependent)
gvk_setup_glslang_target(OSDependent)
gvk_setup_glslang_target(SPIRV)
gvk_setup_glslang_target(SPVRemapper)
