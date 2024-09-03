
list(APPEND gvkCoreSampleLinkLibraries
    gvk-gui
    gvk-handles
    gvk-math
    gvk-runtime
    gvk-spirv
    gvk-structures
    gvk-system
    stb
)

set(gvkCoreSamplesEnabled ${gvk-build-samples})
foreach(gvkCoreSampleLinkLibrary ${gvkCoreSampleLinkLibraries})
    if(NOT ${${gvkCoreSampleLinkLibrary}_ENABLED})
        unset(gvkCoreSamplesEnabled)
    endif()
endforeach()

macro(gvk_add_core_sample target linkLibraries)
    gvk_add_executable(
        TARGET ${target}
        FOLDER "samples/"
        LINK_LIBRARIES ${linkLibraries}
        SOURCE_FILES
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-entry-points.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-entry-points.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-png.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-utilities.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/${target}.cpp"
    )
endmacro()

if(gvkCoreSamplesEnabled)
    gvk_add_core_sample(gvk-getting-started-00-triangle        "${gvkCoreSampleLinkLibraries}")
    gvk_add_core_sample(gvk-getting-started-01-mesh            "${gvkCoreSampleLinkLibraries}")
    gvk_add_core_sample(gvk-getting-started-02-uniform-buffer  "${gvkCoreSampleLinkLibraries}")
    gvk_add_core_sample(gvk-getting-started-03-texture-mapping "${gvkCoreSampleLinkLibraries}")
    gvk_add_core_sample(gvk-getting-started-04-render-target   "${gvkCoreSampleLinkLibraries}")
    gvk_add_core_sample(gvk-getting-started-05-gui             "${gvkCoreSampleLinkLibraries}")
endif()

if(gvk-string_ENABLED)
    gvk_add_executable(
        TARGET gvk-log-parser
        FOLDER "samples/"
        LINK_LIBRARIES gvk-string
        SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/gvk-log-parser.cpp"
    )
endif()

if(gvk-layer_ENABLED)
    gvk_add_layer(
        TARGET VK_LAYER_INTEL_gvk_sample
        FOLDER "samples/"
        LINK_LIBRARIES gvk-layer
        SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-layer.cpp"
        DESCRIPTION "Intel(R) GPA Utilities for Vulkan* sample layer"
    )
endif()
