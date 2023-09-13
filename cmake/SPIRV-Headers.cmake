
include_guard()

include(FetchContent)

set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL  ON CACHE BOOL "" FORCE)
set(SPIRV-Headers_VERSION 124a9665e464ef98b8b718d572d5f329311061eb) # sdk-1.3.261.1
FetchContent_Declare(
    SPIRV-Headers
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Headers.git"
    GIT_TAG ${SPIRV-Headers_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SPIRV-Headers)
gvk_install_artifacts(TARGET SPIRV-Headers VERSION ${SPIRV-Headers_VERSION})
