
include_guard(GLOBAL)

set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL  ON CACHE BOOL "" FORCE)
set(SPIRV-Headers_VERSION 2a611a970fdbc41ac2e3e328802aed9985352dca) # vulkan-sdk-1.4.321.0
FetchContent_Declare(
    SPIRV-Headers
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Headers.git"
    GIT_TAG ${SPIRV-Headers_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SPIRV-Headers)
FetchContent_GetProperties(SPIRV-Headers SOURCE_DIR SPIRV-Headers_SOURCE_DIR)

if(SPIRV-Headers_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET SPIRV-Headers VERSION ${SPIRV-Headers_VERSION})
endif()
if(SPIRV-Headers_INSTALL_HEADERS)
    install(DIRECTORY "${SPIRV-Headers_SOURCE_DIR}/include/spirv/" DESTINATION include/spirv/)
endif()
