
include_guard(GLOBAL)
gvk_enable_target(SPIRV-Headers)

set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL  ON CACHE BOOL "" FORCE)
set(SPIRV-Headers_VERSION 3f17b2af6784bfa2c5aa5dbb8e0e74a607dd8b3b) # vulkan-sdk-1.4.304.0
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
