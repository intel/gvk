
include_guard()

include(FetchContent)

FetchContent_Declare(
    asio
    GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
    GIT_TAG c465349fa5cd91a64bb369f5131ceacab2c0c1c3 # 1.28.0
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(asio)
FetchContent_GetProperties(asio SOURCE_DIR asioSourceDirectory)
add_library(asio_INTERFACE INTERFACE)
target_include_directories(asio_INTERFACE INTERFACE "${asioSourceDirectory}/asio/include/")
