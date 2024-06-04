
include_guard()

include(FetchContent)

set(SPIRV_SKIP_EXECUTABLES   ON CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV-Tools_VERSION dd4b663e13c07fea4fbb3f70c1c91c86731099f7) # vulkan-sdk-1.3.283.0
FetchContent_Declare(
    SPIRV-Tools
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Tools.git"
    GIT_TAG ${SPIRV-Tools_VERSION}
    GIT_PROGRESS TRUE
)

if(CMAKE_FOLDER)
    set(currentCmakeFolder ${CMAKE_FOLDER})
endif()
set(CMAKE_FOLDER "${GVK_IDE_FOLDER}/external/SPIRV-Tools/")
FetchContent_MakeAvailable(SPIRV-Tools)

set_target_properties(spirv-tools-build-version                        PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(SPIRV-Tools-diff                                 PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spirv-tools-header-DebugInfo                     PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spirv-tools-header-NonSemanticShaderDebugInfo100 PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spirv-tools-header-OpenCLDebugInfo100            PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(SPIRV-Tools-link                                 PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(SPIRV-Tools-lint                                 PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(SPIRV-Tools-opt                                  PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(SPIRV-Tools-reduce                               PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(SPIRV-Tools-shared                               PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(SPIRV-Tools-static                               PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spirv-tools-vimsyntax                            PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spv-tools-cldi100                                PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spv-tools-clspvreflection                        PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spv-tools-debuginfo                              PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spv-tools-shdi100                                PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spv-tools-spv-amd-gs                             PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spv-tools-spv-amd-sb                             PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spv-tools-spv-amd-sevp                           PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spv-tools-spv-amd-stm                            PROPERTIES FOLDER "${CMAKE_FOLDER}")
set_target_properties(spv-tools-vkspreflection                         PROPERTIES FOLDER "${CMAKE_FOLDER}")
gvk_install_artifacts(TARGET SPIRV-Tools-opt VERSION ${SPIRV-Tools_VERSION})
gvk_install_artifacts(TARGET SPIRV-Tools-static VERSION ${SPIRV-Tools_VERSION})

# NOTE : Disabling warnings for SPIRV-Tools-shared.  Not using it currently, but
#   there doesn't seem to be an effective way to disable the target using build
#   options or EXCLUDE_FROM_ALL.  If SPIRV-Tools-shared becomes necssary this
#   should be revisted.
# 23>gvk\build\_deps\spirv-tools-src\include\spirv-tools\libspirv.hpp(393,25): error C2220: the following warning is treated as an error
# 28>gvk-getting-started-04-render-target.vcxproj -> gvk\build\samples\Debug\gvk-getting-started-04-render-target.exe
# 23>gvk\build\_deps\spirv-tools-src\include\spirv-tools\libspirv.hpp(393,25): warning C4251: 'spvtools::SpirvTools::impl_': 'std::unique_ptr<spvtools::SpirvTools::Impl,std::default_delete<spvtools::SpirvTools::Impl>>' needs to have dll-interface to be used by clients of 'spvtools::SpirvTools'
target_compile_options(SPIRV-Tools-shared PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/w>)

if(currentCmakeFolder)
    set(CMAKE_FOLDER ${currentCmakeFolder})
else()
    unset(CMAKE_FOLDER)
endif()
