
include_guard(GLOBAL)
gvk_enable_external_module(SPIRV-Headers)

set(SPIRV_SKIP_EXECUTABLES   ON CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV-Tools_VERSION 262bdab48146c937467f826699a40da0fdfc0f1a) # vulkan-sdk-1.4.335.0
FetchContent_Declare(
    SPIRV-Tools
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Tools.git"
    GIT_TAG ${SPIRV-Tools_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(SPIRV-Tools)
FetchContent_GetProperties(SPIRV-Tools SOURCE_DIR SPIRV-Tools_SOURCE_DIR)

# NOTE : Disabling warnings for SPIRV-Tools-shared.  Not using it currently, but
#   there doesn't seem to be an effective way to disable the target using build
#   options or EXCLUDE_FROM_ALL.  If SPIRV-Tools-shared becomes necessary this
#   should be revisted.
# 23>gvk\build\_deps\spirv-tools-src\include\spirv-tools\libspirv.hpp(393,25): error C2220: the following warning is treated as an error
# 28>gvk-getting-started-04-render-target.vcxproj -> gvk\build\samples\Debug\gvk-getting-started-04-render-target.exe
# 23>gvk\build\_deps\spirv-tools-src\include\spirv-tools\libspirv.hpp(393,25): warning C4251: 'spvtools::SpirvTools::impl_': 'std::unique_ptr<spvtools::SpirvTools::Impl,std::default_delete<spvtools::SpirvTools::Impl>>' needs to have dll-interface to be used by clients of 'spvtools::SpirvTools'
target_compile_options(SPIRV-Tools-shared PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/w>)

if(CMAKE_FOLDER)
    set(currentCmakeFolder ${CMAKE_FOLDER})
endif()
set(CMAKE_FOLDER "${GVK_IDE_FOLDER}/external/SPIRV-Tools/")

gvk_get_directory_targets(${SPIRV-Tools_SOURCE_DIR} SPIRV-Tools_TARGETS)
foreach(SPIRV-Tools_TARGET ${SPIRV-Tools_TARGETS})
    string(REPLACE "::" ";" SPIRV-Tools_TARGET ${SPIRV-Tools_TARGET})
    list(LENGTH SPIRV-Tools_TARGET length)
    if(length EQUAL 2)
        list(GET SPIRV-Tools_TARGET 0 SPIRV-Tools_DIRECTORY)
        list(GET SPIRV-Tools_TARGET 1 SPIRV-Tools_TARGET)
        string(REPLACE "${SPIRV-Tools_SOURCE_DIR}" "" SPIRV-Tools_DIRECTORY "${SPIRV-Tools_DIRECTORY}")
        set_target_properties(${SPIRV-Tools_TARGET} PROPERTIES FOLDER "${CMAKE_FOLDER}")
    else()
        message(FATAL_ERROR "${SPIRV-Tools_TARGET} has nonstandard directory structure; standalone configuration required")
    endif()
endforeach()

if(currentCmakeFolder)
    set(CMAKE_FOLDER ${currentCmakeFolder})
else()
    unset(CMAKE_FOLDER)
endif()

if(SPIRV-Tools_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET SPIRV-Tools-opt VERSION ${SPIRV-Tools_VERSION})
    gvk_install_artifacts(TARGET SPIRV-Tools-static VERSION ${SPIRV-Tools_VERSION})
endif()
if(SPIRV-Tools_INSTALL_HEADERS)
    install(DIRECTORY "${SPIRV-Tools_SOURCE_DIR}/include/spirv-tools/" DESTINATION include/spirv-tools/)
endif()
