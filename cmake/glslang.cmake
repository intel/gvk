
include_guard()

include(FetchContent)

set(BUILD_TESTING           OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_INSTALL  OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_JS       OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_WEBMIN   OFF CACHE BOOL "" FORCE)
set(SKIP_GLSLANG_INSTALL    ON  CACHE BOOL "" FORCE)
FetchContent_Declare(
    glslang
    GIT_REPOSITORY "https://github.com/KhronosGroup/glslang.git"
    GIT_TAG d1517d64cfca91f573af1bf7341dc3a5113349c0 # sdk-1.3.250.0
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glslang)
set(folder "${GVK_IDE_FOLDER}/external/glslang/")
set_target_properties(GenericCodeGen     PROPERTIES FOLDER "${folder}")
set_target_properties(glslang            PROPERTIES FOLDER "${folder}")
set_target_properties(MachineIndependent PROPERTIES FOLDER "${folder}")
set_target_properties(OGLCompiler        PROPERTIES FOLDER "${folder}")
set_target_properties(OSDependent        PROPERTIES FOLDER "${folder}")
set_target_properties(SPIRV              PROPERTIES FOLDER "${folder}")
set_target_properties(SPVRemapper        PROPERTIES FOLDER "${folder}")
set_target_properties(HLSL               PROPERTIES FOLDER "${folder}/hlsl/")
add_library(glslang_INTERFACE INTERFACE)
target_link_libraries(
    glslang_INTERFACE
    INTERFACE
        GenericCodeGen
        glslang
        MachineIndependent
        OGLCompiler
        OSDependent
        SPIRV
        SPVRemapper
        HLSL
)
