
include_guard(GLOBAL)

set(imgui_VERSION 4806a1924ff6181180bf5e4b8b79ab4394118875) # 1.91.9b-docking
FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG ${imgui_VERSION}
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui SOURCE_DIR imgui_SOURCE_DIR)

################################################################################
set(ImGuiColorTextEdit_VERSION 165ca5fe8be900884c88b90f16955bbf848b23ee) # `imgui_bundle` 3 March 2025
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

gvk_add_static_library(
    TARGET imgui
    FOLDER "external/"
    INCLUDE_DIRECTORIES
        "${ImGuiColorTextEdit_INCLUDE_DIRECTORIES}"
        "${imgui_SOURCE_DIR}/"
        "${imgui_SOURCE_DIR}/misc/cpp/"
    INCLUDE_FILES
        "${ImGuiColorTextEdit_INCLUDE_FILES}"
        "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h"
        "${imgui_SOURCE_DIR}/imconfig.h"
        "${imgui_SOURCE_DIR}/imgui.h"
        "${imgui_SOURCE_DIR}/imgui_internal.h"
        "${imgui_SOURCE_DIR}/imstb_rectpack.h"
        "${imgui_SOURCE_DIR}/imstb_textedit.h"
        "${imgui_SOURCE_DIR}/imstb_truetype.h"
    SOURCE_FILES
        "${ImGuiColorTextEdit_SOURCE_FILES}"
        "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
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
