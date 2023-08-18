
include_guard()

include(FetchContent)

set(asio_VERSION c465349fa5cd91a64bb369f5131ceacab2c0c1c3) # 1.28.0
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
