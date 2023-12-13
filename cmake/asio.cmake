
include_guard()

include(FetchContent)

set(asio_VERSION 814f67e730e154547aea3f4d99f709cbdf1ea4a0) # 1.29.0
FetchContent_Declare(
    asio
    GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
    GIT_TAG ${asio_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(asio)
FetchContent_GetProperties(asio SOURCE_DIR asio_SOURCE_DIR)
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE "$<BUILD_INTERFACE:${asio_SOURCE_DIR}/asio/include/>" $<INSTALL_INTERFACE:include>)
gvk_install_artifacts(TARGET asio VERSION ${asio_VERSION})
