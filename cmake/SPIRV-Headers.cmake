
include_guard()

include(FetchContent)

set(SPIRV_HEADERS_SKIP_EXAMPLES ON  CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL  ON  CACHE BOOL "" FORCE)
FetchContent_Declare(
    SPIRV-Headers
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Headers.git"
    GIT_TAG d13b52222c39a7e9a401b44646f0ca3a640fbd47 # sdk-1.3.239.0
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SPIRV-Headers)
set(folder "${GVK_IDE_FOLDER}/external/SPIRV-Headers/")
