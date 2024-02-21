
macro(gvk_add_sample target)
    gvk_add_executable(
        TARGET ${target}
        FOLDER "samples/"
        LINK_LIBRARIES gvk gvk-xml stb
        SOURCE_FILES
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-entry-points.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-entry-points.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-png.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-utilities.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/${target}.cpp"
        COMPILE_DEFINITIONS
            GVK_XML_FILE_PATH="${Vulkan_XML}"
    )
endmacro()

gvk_add_sample(gvk-getting-started-00-triangle)
gvk_add_sample(gvk-getting-started-01-mesh)
gvk_add_sample(gvk-getting-started-02-uniform-buffer)
gvk_add_sample(gvk-getting-started-03-texture-mapping)
gvk_add_sample(gvk-getting-started-04-render-target)
gvk_add_sample(gvk-getting-started-05-gui)
gvk_add_sample(gvk-log-parser)

if(GVK_BUILD_LAYER)
    gvk_add_layer(
        TARGET VK_LAYER_INTEL_gvk_sample
        FOLDER "samples/"
        LINK_LIBRARIES gvk
        SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-layer.cpp"
        DESCRIPTION "Intel(R) GPA Utilities for Vulkan* sample layer"
    )
endif()
