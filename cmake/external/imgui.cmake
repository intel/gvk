

include_guard(GLOBAL)
gvk_enable_target(imgui)

set(imgui_VERSION 993fa347495860ed44b83574254ef2a317d0c14f) # 1.91.6
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
    INCLUDE_DIRECTORIES
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/misc/"
    INCLUDE_FILES
        "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h"
        "${imgui_SOURCE_DIR}/imconfig.h"
        "${imgui_SOURCE_DIR}/imgui.h"
        "${imgui_SOURCE_DIR}/imgui_internal.h"
        "${imgui_SOURCE_DIR}/imstb_rectpack.h"
        "${imgui_SOURCE_DIR}/imstb_textedit.h"
        "${imgui_SOURCE_DIR}/imstb_truetype.h"
    SOURCE_FILES
        "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
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
            "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h"
            "${imgui_SOURCE_DIR}/imconfig.h"
            "${imgui_SOURCE_DIR}/imgui.h"
            "${imgui_SOURCE_DIR}/imgui_internal.h"
        DESTINATION
            include/
    )
endif()
