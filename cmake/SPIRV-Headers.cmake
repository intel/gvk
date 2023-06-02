
include_guard()

include(FetchContent)

set(SPIRV_HEADERS_SKIP_EXAMPLES ON  CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL  ON  CACHE BOOL "" FORCE)
FetchContent_Declare(
    SPIRV-Headers
    GIT_REPOSITORY "https://github.com/KhronosGroup/SPIRV-Headers.git"
    GIT_TAG 1feaf4414eb2b353764d01d88f8aa4bcc67b60db # sdk-1.3.243.0
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SPIRV-Headers)
set(folder "${GVK_IDE_FOLDER}/external/SPIRV-Headers/")
