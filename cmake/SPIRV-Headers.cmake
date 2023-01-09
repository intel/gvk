
include_guard()

include(FetchContent)

set(SPIRV_HEADERS_SKIP_EXAMPLES ON  CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL  ON  CACHE BOOL "" FORCE)
FetchContent_Declare(
    SPIRV-Headers
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Headers.git"
    GIT_TAG 85a1ed200d50660786c1a88d9166e871123cce39 # sdk-1.3.231.1
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SPIRV-Headers)
set(folder "${GVK_IDE_FOLDER}/external/SPIRV-Headers/")
