
set(BUILD_TESTING           OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_INSTALL  OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_JS       OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_WEBMIN   OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glslang
    GIT_REPOSITORY "https://github.com/KhronosGroup/glslang.git"
    GIT_TAG adbf0d3106b26daa237b10b9bf72b1af7c31092d # 11.10.0
    GIT_PROGRESS TRUE
    FETCHCONTENT_UPDATES_DISCONNECTED
)
FetchContent_MakeAvailable(glslang)
set(folder "gvk/external/glslang/")
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
