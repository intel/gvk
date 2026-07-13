
################################################################################
# gvk-pipeline-explorer
list(APPEND linkLibraries
    gvk-gui
    gvk-pipeline-explorer-backend
)
if(GVK_AUTOCORR_ENABLED)
    list(APPEND linkLibraries gvk-autocorr)
endif()
if(GVK_PYTHON_ENABLED)
    list(APPEND linkLibraries gvk-python)
endif()

gvk_add_executable(
    TARGET
        gvk-pipeline-explorer
    FOLDER
        "gvk-pipeline-explorer/"
    LINK_LIBRARIES
        ${linkLibraries}
    INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_LIST_DIR}/gui/"
    INCLUDE_FILES
        "${includePath}/gui/metrics/autocorr-window.hpp"
        "${includePath}/gui/metrics/metrics-charts-window.hpp"
        "${includePath}/gui/metrics/metrics-tab.hpp"
        "${includePath}/gui/metrics/metrics-window.hpp"
        "${includePath}/gui/metrics/performance-query-metrics-tab.hpp"
        "${includePath}/gui/metrics/pipeline-statistics-metrics-tab.hpp"
        "${includePath}/gui/metrics/plugin-metrics-tab.hpp"
        "${includePath}/gui/api-call-explorer-window.hpp"
        "${includePath}/gui/api-call-timeline-window.hpp"
        "${includePath}/gui/console-window.hpp"
        "${includePath}/gui/file-window.hpp"
        "${includePath}/gui/gui-info.hpp"
        "${includePath}/gui/image-window.hpp"
        "${includePath}/gui/launch-options.hpp"
        "${includePath}/gui/pipeline-info.hpp"
        "${includePath}/gui/pipelines-window.hpp"
        "${includePath}/gui/range-info.hpp"
        "${includePath}/gui/selected-pipeline-window.hpp"
        "${includePath}/gui/stream-playback-window.hpp"
        "${includePath}/gui/window-manager.hpp"
        "${includePath}/gui/window.hpp"
        "${includePath}/gui/workspace-window.hpp"
    SOURCE_FILES
        "${sourcePath}/gui/metrics/autocorr-window.cpp"
        "${sourcePath}/gui/metrics/metrics-charts-window.cpp"
        "${sourcePath}/gui/metrics/metrics-tab.cpp"
        "${sourcePath}/gui/metrics/metrics-window.cpp"
        "${sourcePath}/gui/metrics/performance-query-metrics-tab.cpp"
        "${sourcePath}/gui/metrics/pipeline-statistics-metrics-tab.cpp"
        "${sourcePath}/gui/metrics/plugin-metrics-tab.cpp"
        "${sourcePath}/gui/api-call-explorer-window.cpp"
        "${sourcePath}/gui/api-call-timeline-window.cpp"
        "${sourcePath}/gui/console-window.cpp"
        "${sourcePath}/gui/file-window.cpp"
        "${sourcePath}/gui/main.cpp"
        "${sourcePath}/gui/image-window.cpp"
        "${sourcePath}/gui/launch-options.cpp"
        "${sourcePath}/gui/pipelines-window.cpp"
        "${sourcePath}/gui/range-info.cpp"
        "${sourcePath}/gui/selected-pipeline-window.cpp"
        "${sourcePath}/gui/stream-playback-window.cpp"
        "${sourcePath}/gui/window-manager.cpp"
        "${sourcePath}/gui/window.cpp"
        "${sourcePath}/gui/workspace-window.cpp"
)

add_custom_command(
    TARGET gvk-pipeline-explorer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/fonts/fa-solid-900.ttf" "$<TARGET_FILE_DIR:gvk-pipeline-explorer>/../"
)
