
include_guard()

include(FetchContent)

set(SPIRV_SKIP_EXECUTABLES   ON  CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON  CACHE BOOL "" FORCE)
FetchContent_Declare(
    SPIRV-Tools
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Tools.git"
    GIT_TAG 44d72a9b36702f093dd20815561a56778b2d181e # sdk-1.3.243.0
    GIT_PROGRESS TRUE
)
if(CMAKE_FOLDER)
    set(currentCmakeFolder ${CMAKE_FOLDER})
endif()
set(CMAKE_FOLDER "${GVK_IDE_FOLDER}/external/SPIRV-Tools/")
FetchContent_MakeAvailable(SPIRV-Tools)
set_target_properties(spirv-tools-build-version                        PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(spirv-tools-header-DebugInfo                     PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(spirv-tools-header-NonSemanticShaderDebugInfo100 PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(spirv-tools-header-OpenCLDebugInfo100            PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(spv-tools-cldi100                                PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(spv-tools-clspvreflection                        PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(spv-tools-debuginfo                              PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(spv-tools-shdi100                                PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(spv-tools-spv-amd-gs                             PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(spv-tools-spv-amd-sb                             PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(spv-tools-spv-amd-sevp                           PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(spv-tools-spv-amd-stm                            PROPERTIES FOLDER "${CMAKE_FOLDER}/build/")
set_target_properties(SPIRV-Tools-diff                                 PROPERTIES FOLDER "${CMAKE_FOLDER}/libraries/")
set_target_properties(SPIRV-Tools-link                                 PROPERTIES FOLDER "${CMAKE_FOLDER}/libraries/")
set_target_properties(SPIRV-Tools-lint                                 PROPERTIES FOLDER "${CMAKE_FOLDER}/libraries/")
set_target_properties(SPIRV-Tools-opt                                  PROPERTIES FOLDER "${CMAKE_FOLDER}/libraries/")
set_target_properties(SPIRV-Tools-reduce                               PROPERTIES FOLDER "${CMAKE_FOLDER}/libraries/")
set_target_properties(SPIRV-Tools-shared                               PROPERTIES FOLDER "${CMAKE_FOLDER}/libraries/")
set_target_properties(SPIRV-Tools-static                               PROPERTIES FOLDER "${CMAKE_FOLDER}/libraries/")
set_target_properties(spirv-tools-vimsyntax                            PROPERTIES FOLDER "${CMAKE_FOLDER}/utilities/")
if(currentCmakeFolder)
    set(CMAKE_FOLDER ${currentCmakeFolder})
else()
    unset(CMAKE_FOLDER)
endif()
