
include_guard()

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG d7c8516a4b848c0291e3d75b627c0843f515f591 # 1.89.2
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui SOURCE_DIR imguiSourceDirectory)

gvk_add_static_library(
    target imgui
    folder "external/"
    includeDirectories "${imguiSourceDirectory}"
    includeFiles
        "${imguiSourceDirectory}/imconfig.h"
        "${imguiSourceDirectory}/imgui.h"
        "${imguiSourceDirectory}/imgui_internal.h"
        "${imguiSourceDirectory}/imstb_rectpack.h"
        "${imguiSourceDirectory}/imstb_textedit.h"
        "${imguiSourceDirectory}/imstb_truetype.h"
    sourceFiles
        "${imguiSourceDirectory}/imgui.cpp"
        "${imguiSourceDirectory}/imgui_demo.cpp"
        "${imguiSourceDirectory}/imgui_draw.cpp"
        "${imguiSourceDirectory}/imgui_tables.cpp"
        "${imguiSourceDirectory}/imgui_widgets.cpp"
)
