
include_guard()

include(FetchContent)

set(tinyxml2_VERSION 321ea883b7190d4e85cae5512a12e5eaa8f8731f) # 10.0.0
FetchContent_Declare(
    tinyxml2
    GIT_REPOSITORY "https://github.com/leethomason/tinyxml2.git"
    GIT_TAG ${tinyxml2_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_GetProperties(tinyxml2)
if(NOT tinyxml2_POPULATED)
    FetchContent_Populate(tinyxml2)
    file(COPY "${tinyxml2_SOURCE_DIR}/tinyxml2.h" DESTINATION "${tinyxml2_BINARY_DIR}/tinyxml2/")
    file(COPY "${tinyxml2_SOURCE_DIR}/tinyxml2.cpp" DESTINATION "${tinyxml2_BINARY_DIR}/tinyxml2/")
    gvk_add_static_library(
        TARGET tinyxml2
        FOLDER "external/"
        INCLUDE_DIRECTORIES "${tinyxml2_BINARY_DIR}/"
        INCLUDE_FILES "${tinyxml2_BINARY_DIR}/tinyxml2/tinyxml2.h"
        SOURCE_FILES "${tinyxml2_BINARY_DIR}/tinyxml2/tinyxml2.cpp"
    )
    gvk_install_library(TARGET tinyxml2 VERSION ${tinyxml2_VERSION})
endif()
