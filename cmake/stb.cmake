
FetchContent_Declare(
    stb
    GIT_REPOSITORY "https://github.com/nothings/stb.git"
    GIT_TAG af1a5bc352164740c1cc1354942b1c6b72eacb8a
    GIT_PROGRESS TRUE
    FETCHCONTENT_UPDATES_DISCONNECTED
)
FetchContent_MakeAvailable(stb)
FetchContent_GetProperties(stb SOURCE_DIR stbSourceDirectory)
FetchContent_GetProperties(stb BINARY_DIR stbBinaryDirectory)

macro(add_stb_file stbFile implementationMacro)
    file(COPY "${stbSourceDirectory}/${stbFile}.h" DESTINATION "${stbBinaryDirectory}/stb/")
    list(APPEND includeFiles "${stbBinaryDirectory}/stb/${stbFile}.h")
    set(sourceFile "${stbBinaryDirectory}/stb/${stbFile}.cpp")
    list(APPEND sourceFiles "${sourceFile}")
    if(NOT EXISTS "${sourceFile}")
        file(WRITE "${sourceFile}" "\n#define ${implementationMacro}\n#include \"${stbFile}.h\"\n")
    endif()
endmacro()

add_stb_file(stb_image STB_IMAGE_IMPLEMENTATION)

gvk_add_static_library(
    target stb
    folder "external/"
    includeDirectories "${stbBinaryDirectory}/"
    includeFiles "${includeFiles}"
    sourceFiles "${sourceFiles}"
)
