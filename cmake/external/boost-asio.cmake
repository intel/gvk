
include_guard(GLOBAL)
find_package(Threads REQUIRED)

set(boost-asio_VERSION 03ae834edbace31a96157b89bf50e5ee464e5ef9) # 1.32.0
FetchContent_Declare(
    boost-asio
    GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
    GIT_TAG ${boost-asio_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(boost-asio)
FetchContent_GetProperties(boost-asio SOURCE_DIR boost-asio_SOURCE_DIR)
add_library(boost-asio INTERFACE)
target_include_directories(boost-asio INTERFACE "$<BUILD_INTERFACE:${boost-asio_SOURCE_DIR}/asio/include/>" $<INSTALL_INTERFACE:include>)

if(boost-asio_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET boost-asio VERSION ${boost-asio_VERSION})
endif()
if(boost-asio_INSTALL_HEADERS)
    gvk_install_headers(TARGET boost-asio EXCLUDE ".gitignore;Makefile.am")
endif()
