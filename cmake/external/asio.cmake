
include_guard(GLOBAL)
gvk_enable_target(asio)
find_package(Threads REQUIRED)

set(asio_VERSION 03ae834edbace31a96157b89bf50e5ee464e5ef9) # 1.32.0
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

if(gvk-asio_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET asio VERSION ${asio_VERSION})
endif()
if(gvk-asio_INSTALL_HEADERS)
    gvk_install_headers(TARGET asio EXCLUDE ".gitignore;Makefile.am")
endif()
