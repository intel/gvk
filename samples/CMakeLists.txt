
macro(gvk_add_sample target)
    gvk_add_executable(
        target ${target}
        folder "samples/"
        linkLibraries gvk stb
        sourceFiles
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-entry-points.cpp"
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-entry-points.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-png.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-utilities.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/${target}.cpp"
    )
endmacro()

gvk_add_sample(gvk-getting-started-00-triangle)
gvk_add_sample(gvk-getting-started-01-mesh)
gvk_add_sample(gvk-getting-started-02-uniform-buffer)
gvk_add_sample(gvk-getting-started-03-texture-mapping)
gvk_add_sample(gvk-getting-started-04-render-target)
gvk_add_sample(gvk-getting-started-05-gui)

if(GVK_BUILD_LAYER)
    gvk_add_layer(
        target VK_LAYER_INTEL_gvk_sample
        folder "samples/"
        linkLibraries gvk
        sourceFiles "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-layer.cpp"
        description "Intel(R) GPA Utilities for Vulkan* sample layer"
    )
endif()
