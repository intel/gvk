
include_guard(GLOBAL)
gvk_enable_target(glslang)
include(external/SPIRV-Tools)

set(BUILD_TESTING           OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_INSTALL  OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_JS       OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_WEBMIN   OFF CACHE BOOL "" FORCE)
set(SKIP_GLSLANG_INSTALL    ON  CACHE BOOL "" FORCE)
set(glslang_VERSION fa9c3deb49e035a8abcabe366f26aac010f6cbfb) # vulkan-sdk-1.3.290.0
FetchContent_Declare(
    glslang
    GIT_REPOSITORY "https://github.com/KhronosGroup/glslang.git"
    GIT_TAG ${glslang_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(glslang)

# HACK : glslang headers aren't being installed, but the export expects them to
#   be at `include/External`...this modifies the INSTALL_INTERFACE to `include`
#   to avoid an error for missing INTERFACE_INCLUDE_DIRECTORIES on import.
get_target_property(SPIRV_INTERFACE_INCLUDE_DIRECTORIES SPIRV INTERFACE_INCLUDE_DIRECTORIES)
string(REPLACE "$<INSTALL_INTERFACE:include/External>" "$<INSTALL_INTERFACE:include>" SPIRV_INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_INTERFACE_INCLUDE_DIRECTORIES}")
set_target_properties(SPIRV PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_INTERFACE_INCLUDE_DIRECTORIES}")

macro(gvk_setup_glslang_target glslangTarget)
    list(APPEND glslangLibraries ${glslangTarget})
    set_target_properties(${glslangTarget} PROPERTIES FOLDER "${GVK_IDE_FOLDER}/external/glslang/")
    if(gvk-glslang_INSTALL_ARTIFACTS)
        gvk_install_artifacts(TARGET ${glslangTarget} VERSION ${glslang_VERSION})
    endif()
    if(gvk-glslang_INSTALL_HEADERS)
        # TODO :
    endif()
endmacro()

gvk_setup_glslang_target(GenericCodeGen)
gvk_setup_glslang_target(glslang)
gvk_setup_glslang_target(glslang-default-resource-limits)
gvk_setup_glslang_target(MachineIndependent)
gvk_setup_glslang_target(OSDependent)
gvk_setup_glslang_target(SPIRV)
gvk_setup_glslang_target(SPVRemapper)
set(glslangLibraries ${glslangLibraries} PARENT_SCOPE)
