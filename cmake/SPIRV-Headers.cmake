
include_guard()

include(FetchContent)

set(SPIRV_HEADERS_SKIP_EXAMPLES ON  CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL  ON  CACHE BOOL "" FORCE)
FetchContent_Declare(
    SPIRV-Headers
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Headers.git"
    GIT_TAG 268a061764ee69f09a477a695bf6a11ffe311b8d # sdk-1.3.250.0
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SPIRV-Headers)
set(folder "${GVK_IDE_FOLDER}/external/SPIRV-Headers/")
