
include_guard(GLOBAL)
gvk_enable_target(SPIRV-Headers)

set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL  ON CACHE BOOL "" FORCE)
set(SPIRV-Headers_VERSION 2a9b6f951c7d6b04b6c21fe1bf3f475b68b84801) # vulkan-sdk-1.3.296.0
FetchContent_Declare(
    SPIRV-Headers
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Headers.git"
    GIT_TAG ${SPIRV-Headers_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(SPIRV-Headers)
FetchContent_GetProperties(SPIRV-Headers SOURCE_DIR SPIRV-Headers_SOURCE_DIR)

if(gvk-SPIRV-Headers_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET SPIRV-Headers VERSION ${SPIRV-Headers_VERSION})
endif()
if(gvk-SPIRV-Headers_INSTALL_HEADERS)
    install(DIRECTORY "${SPIRV-Headers_SOURCE_DIR}/include/spirv/" DESTINATION include/spirv/)
endif()
