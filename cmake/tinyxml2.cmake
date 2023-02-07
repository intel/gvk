
include_guard()

include(FetchContent)

FetchContent_Declare(
    tinyxml2_content
    GIT_REPOSITORY "https://github.com/leethomason/tinyxml2.git"
    GIT_TAG 1dee28e51f9175a31955b9791c74c430fe13dc82 # 9.0.0
    GIT_PROGRESS TRUE
)
FetchContent_GetProperties(tinyxml2_content)
if(NOT tinyxml2_content_POPULATED)
    FetchContent_Populate(tinyxml2_content)
    file(COPY "${tinyxml2_content_SOURCE_DIR}/tinyxml2.h" DESTINATION "${tinyxml2_content_BINARY_DIR}/tinyxml2/")
    file(COPY "${tinyxml2_content_SOURCE_DIR}/tinyxml2.cpp" DESTINATION "${tinyxml2_content_BINARY_DIR}/tinyxml2/")
    gvk_add_static_library(
        target tinyxml2
        folder "external/"
        includeDirectories "${tinyxml2_content_BINARY_DIR}/"
        includeFiles "${tinyxml2_content_BINARY_DIR}/tinyxml2/tinyxml2.h"
        sourceFiles "${tinyxml2_content_BINARY_DIR}/tinyxml2/tinyxml2.cpp"
    )
endif()
