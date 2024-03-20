
include_guard()

include(FetchContent)

set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL  ON CACHE BOOL "" FORCE)
set(SPIRV-Headers_VERSION 1c6bb2743599e6eb6f37b2969acc0aef812e32e3) # vulkan-sdk-1.3.275.0
FetchContent_Declare(
    SPIRV-Headers
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Headers.git"
    GIT_TAG ${SPIRV-Headers_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SPIRV-Headers)
gvk_install_artifacts(TARGET SPIRV-Headers VERSION ${SPIRV-Headers_VERSION})
