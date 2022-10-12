
FetchContent_Declare(
    asio
    GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
    GIT_TAG bba12d10501418fd3789ce01c9f86a77d37df7ed # 1.22.1
    GIT_PROGRESS TRUE
    FETCHCONTENT_UPDATES_DISCONNECTED
)
FetchContent_MakeAvailable(asio)
FetchContent_GetProperties(asio SOURCE_DIR asioSourceDirectory)
add_library(asio_INTERFACE INTERFACE)
target_include_directories(asio_INTERFACE INTERFACE "${asioSourceDirectory}/asio/include/")
