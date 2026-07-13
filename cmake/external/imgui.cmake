
include_guard(GLOBAL)

# NOTE : There's a newer version of ImGui available, but it introduces some
#   breaking changes in ImPlot which hasn't been updated to latest ImGui yet
set(imgui_VERSION 3912b3d9a9c1b3f17431aebafd86d2f40ee6e59c) # v1.92.5-docking
FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG ${imgui_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui SOURCE_DIR imgui_SOURCE_DIR)

################################################################################
set(ImGuiColorTextEdit_VERSION e3f369f2afed51087fe43c4ad97c33e5b440ed47) # `imgui_bundle` 16 Sep 2025
FetchContent_Declare(
    ImGuiColorTextEdit
    GIT_REPOSITORY "https://github.com/pthom/ImGuiColorTextEdit.git"
    GIT_TAG ${ImGuiColorTextEdit_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(ImGuiColorTextEdit)
FetchContent_GetProperties(ImGuiColorTextEdit SOURCE_DIR ImGuiColorTextEdit_SOURCE_DIR)
set(ImGuiColorTextEdit_INCLUDE_DIRECTORIES
    "${ImGuiColorTextEdit_SOURCE_DIR}/"
    "${ImGuiColorTextEdit_SOURCE_DIR}/vendor/regex/include/"
)
set(ImGuiColorTextEdit_INCLUDE_FILES
    "${ImGuiColorTextEdit_SOURCE_DIR}/TextEditor.h"
)
set(ImGuiColorTextEdit_SOURCE_FILES
    "${ImGuiColorTextEdit_SOURCE_DIR}/ImGuiDebugPanel.cpp"
    "${ImGuiColorTextEdit_SOURCE_DIR}/LanguageDefinitions.cpp"
    "${ImGuiColorTextEdit_SOURCE_DIR}/TextEditor.cpp"
)
################################################################################
set(implot_VERSION 2babf8b1bdfa9fd01cf5f401d10abb506bd504a5) # `imgui_bundle` 25 Nov 2025
FetchContent_Declare(
    implot
    GIT_REPOSITORY "https://github.com/pthom/implot.git"
    GIT_TAG ${implot_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(implot)
FetchContent_GetProperties(implot SOURCE_DIR implot_SOURCE_DIR)
set(implot_INCLUDE_DIRECTORIES
    "${implot_SOURCE_DIR}/"
)
set(implot_INCLUDE_FILES
    "${implot_SOURCE_DIR}/implot.h"
)
set(implot_SOURCE_FILES
    "${implot_SOURCE_DIR}/implot.cpp"
    "${implot_SOURCE_DIR}/implot_demo.cpp"
    "${implot_SOURCE_DIR}/implot_internal.h"
    "${implot_SOURCE_DIR}/implot_items.cpp"
)
################################################################################
# https://github.com/FortAwesome/Font-Awesome/blob/6.x/webfonts/fa-brands-400.ttf
FetchContent_Declare(
    Font-Awesome
    URL https://github.com/FortAwesome/Font-Awesome/raw/6.x/webfonts/fa-solid-900.ttf
    DOWNLOAD_NO_EXTRACT TRUE
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/fonts/
)
FetchContent_MakeAvailable(Font-Awesome)
################################################################################

gvk_add_static_library(
    TARGET imgui
    FOLDER "external/"
    INCLUDE_DIRECTORIES
        "${ImGuiColorTextEdit_INCLUDE_DIRECTORIES}"
        "${implot_INCLUDE_DIRECTORIES}"
        "${imgui_SOURCE_DIR}/"
        "${imgui_SOURCE_DIR}/misc/cpp/"
    INCLUDE_FILES
        "${ImGuiColorTextEdit_INCLUDE_FILES}"
        "${implot_INCLUDE_FILES}"
        "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h"
        "${imgui_SOURCE_DIR}/imconfig.h"
        "${imgui_SOURCE_DIR}/imgui.h"
        "${imgui_SOURCE_DIR}/imgui_internal.h"
        "${imgui_SOURCE_DIR}/imstb_rectpack.h"
        "${imgui_SOURCE_DIR}/imstb_textedit.h"
        "${imgui_SOURCE_DIR}/imstb_truetype.h"
    SOURCE_FILES
        "${ImGuiColorTextEdit_SOURCE_FILES}"
        "${implot_SOURCE_FILES}"
        "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    COMPILE_DEFINITIONS
        IMGUI_DISABLE_STB_IMAGE_WRITE_IMPLEMENTATION=1
)

target_compile_options(imgui PRIVATE -w)

# if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
# endif()
# if(MSVC)
#     target_compile_options(imgui PRIVATE -w)
# else()
#     set_source_files_properties("${imgui_SOURCE_DIR}/imgui.cpp" PROPERTIES COMPILE_FLAGS "-Wno-strict-aliasing")
#     set_source_files_properties(
#         ${ImGuiColorTextEdit_SOURCE_FILES}
#         PROPERTIES COMPILE_FLAGS "-Wno-sign-compare -Wno-parentheses -Wno-unused-parameter -Wno-unused-variable -Wno-maybe-uninitialized"
#     )
# endif()

if(imgui_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET imgui VERSION ${imgui_VERSION})
    install(FILES "${CMAKE_BINARY_DIR}/fonts/fa-solid-900.ttf" DESTINATION bin/$<CONFIG>/)
endif()
if(imgui_INSTALL_HEADERS)
    install(
        FILES
            "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h"
            "${imgui_SOURCE_DIR}/imconfig.h"
            "${imgui_SOURCE_DIR}/imgui.h"
            "${imgui_SOURCE_DIR}/imgui_internal.h"
        DESTINATION
            include/
    )
endif()
