
include_guard(GLOBAL)
find_package(Threads REQUIRED)

set(boost-multiprecision_VERSION c48ae180f5b22fca8479ac7e0c5f0d527d442072) # Boost_1_89_0
FetchContent_Declare(
    boost-multiprecision
    GIT_REPOSITORY "https://github.com/boostorg/multiprecision.git"
    GIT_TAG ${boost-multiprecision_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(boost-multiprecision)
FetchContent_GetProperties(boost-multiprecision SOURCE_DIR boost-multiprecision_SOURCE_DIR)
add_library(boost-multiprecision INTERFACE)
target_include_directories(boost-multiprecision INTERFACE "$<BUILD_INTERFACE:${boost-multiprecision_SOURCE_DIR}/include/>" $<INSTALL_INTERFACE:include>)

if(boost-multiprecision_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET boost-multiprecision VERSION ${boost-multiprecision_VERSION})
endif()
if(boost-multiprecision_INSTALL_HEADERS)
    gvk_install_headers(TARGET boost-multiprecision)
endif()
