
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
    if(VK_NO_PROTOTYPES)
        target_compile_definitions(${target} PUBLIC VK_NO_PROTOTYPES=1)
    endif()
endmacro()

if(gvkCoreSamplesEnabled)
    # NOTE : Samples set to 896 x 504 for screenshots
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

if(gvk-xml_ENABLED)
    gvk_add_executable(
        TARGET gvk-xml-compare
        FOLDER "samples/"
        LINK_LIBRARIES gvk-xml
        SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/gvk-xml-compare.cpp"
    )
    gvk_add_executable(
        TARGET gvk-xml-search
        FOLDER "samples/"
        LINK_LIBRARIES gvk-xml
        SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/gvk-xml-search.cpp"
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

if(gvk-state-tracker_ENABLED AND MSVC)
    gvk_add_executable(
        TARGET gvk-metadata-extractor-sample
        FOLDER "samples/gvk-metadata-extractor-sample/"
        LINK_LIBRARIES
            gvk-command-structures
            gvk-gui
            gvk-handles
            gvk-system
            VK_LAYER_INTEL_gvk_state_tracker_metadata_extractor
        INCLUDE_FILES
            "${CMAKE_CURRENT_LIST_DIR}/gvk-metadata-extractor-sample-utilities.hpp"
        SOURCE_FILES
            "${CMAKE_CURRENT_LIST_DIR}/gvk-metadata-extractor-sample.cpp"
    )
    gvk_add_layer(
        TARGET VK_LAYER_INTEL_gvk_metadata_extractor_sample
        FOLDER "samples/gvk-metadata-extractor-sample/"
        LINK_LIBRARIES
            gvk-containers
            gvk-handles
            VK_LAYER_INTEL_gvk_state_tracker_metadata_extractor
        INCLUDE_FILES
            "${CMAKE_CURRENT_LIST_DIR}/gvk-metadata-extractor-sample-utilities.hpp"
        SOURCE_FILES
            "${CMAKE_CURRENT_LIST_DIR}/gvk-metadata-extractor-sample-layer.cpp"
        DESCRIPTION
            "Intel(R) GPA Utilities for Vulkan* metadata extractor sample layer"
    )
    add_custom_command(
        TARGET gvk-metadata-extractor-sample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:VK_LAYER_INTEL_gvk_state_tracker>" "$<TARGET_FILE_DIR:gvk-metadata-extractor-sample>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:VK_LAYER_INTEL_gvk_state_tracker>/VK_LAYER_INTEL_gvk_state_tracker.json" "$<TARGET_FILE_DIR:gvk-metadata-extractor-sample>"
    )
endif()
