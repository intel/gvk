

include_guard(GLOBAL)
gvk_enable_target(imgui)

set(imgui_VERSION 3232070d3abaec531d75e060fab14a26cb36ab55) # 1.91.0 + NOTE : This commit is ahead of the latest tagged release, it clears a GCC warning for format-truncation
FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG ${imgui_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui SOURCE_DIR imgui_SOURCE_DIR)
gvk_add_static_library(
    TARGET imgui
    FOLDER "external/"
    INCLUDE_DIRECTORIES "${imgui_SOURCE_DIR}"
    INCLUDE_FILES
        "${imgui_SOURCE_DIR}/imconfig.h"
        "${imgui_SOURCE_DIR}/imgui.h"
        "${imgui_SOURCE_DIR}/imgui_internal.h"
        "${imgui_SOURCE_DIR}/imstb_rectpack.h"
        "${imgui_SOURCE_DIR}/imstb_textedit.h"
        "${imgui_SOURCE_DIR}/imstb_truetype.h"
    SOURCE_FILES
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_source_files_properties("${imgui_SOURCE_DIR}/imgui.cpp" PROPERTIES COMPILE_FLAGS "-Wno-strict-aliasing")
endif()

if(gvk-imgui_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET imgui VERSION ${imgui_VERSION})
endif()
if(gvk-imgui_INSTALL_HEADERS)
    install(
        FILES
            "${imgui_SOURCE_DIR}/imconfig.h"
            "${imgui_SOURCE_DIR}/imgui.h"
        DESTINATION
            include/
    )
endif()
