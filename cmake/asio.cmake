
include_guard()

include(FetchContent)

FetchContent_Declare(
    asio
    GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
    GIT_TAG 147f7225a96d45a2807a64e443177f621844e51c # 1.24.0
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(asio)
FetchContent_GetProperties(asio SOURCE_DIR asioSourceDirectory)
add_library(asio_INTERFACE INTERFACE)
target_include_directories(asio_INTERFACE INTERFACE "${asioSourceDirectory}/asio/include/")
