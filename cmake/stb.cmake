
include_guard()

include(FetchContent)

set(stb_VERSION f4a71b13373436a2866c5d68f8f80ac6f0bc1ffe)
FetchContent_Declare(
    stb
    GIT_REPOSITORY "https://github.com/nothings/stb.git"
    GIT_TAG ${stb_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(stb)
FetchContent_GetProperties(stb SOURCE_DIR stb_SOURCE_DIR)
FetchContent_GetProperties(stb BINARY_DIR stb_BINARY_DIR)

macro(gvk_add_stb_file stbFile implementationMacro)
    file(COPY "${stb_SOURCE_DIR}/${stbFile}.h" DESTINATION "${stb_BINARY_DIR}/stb/")
    list(APPEND includeFiles "${stb_BINARY_DIR}/stb/${stbFile}.h")
    set(sourceFile "${stb_BINARY_DIR}/stb/${stbFile}.cpp")
    list(APPEND sourceFiles "${sourceFile}")
    if(NOT EXISTS "${sourceFile}")
        file(WRITE "${sourceFile}"
"
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored \"-Wmissing-field-initializers\"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored \"-Wmissing-field-initializers\"
#elif defined(_MSVC_LANG)
#pragma warning(push, 0)
#endif

#define ${implementationMacro}
#include \"${stbFile}.h\"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSVC_LANG)
#pragma warning(pop)
#endif
"
        )
    endif()
endmacro()

gvk_add_stb_file(stb_image STB_IMAGE_IMPLEMENTATION)
gvk_add_stb_file(stb_image_write STB_IMAGE_WRITE_IMPLEMENTATION)

gvk_add_static_library(
    TARGET stb
    FOLDER "external/"
    INCLUDE_DIRECTORIES "${stb_BINARY_DIR}/"
    INCLUDE_FILES "${includeFiles}"
    SOURCE_FILES "${sourceFiles}"
)

gvk_install_artifacts(TARGET stb VERSION ${stb_VERSION})
