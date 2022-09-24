
macro(gvk_add_sample target)
    gvk_add_executable(
        target ${target}
        folder "samples/"
        linkLibraries gvk
        sourceFiles
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-png.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/gvk-sample-utilities.hpp"
            "${CMAKE_CURRENT_LIST_DIR}/${target}.cpp"
    )
endmacro()

gvk_add_sample(getting-started-00-triangle)
gvk_add_sample(getting-started-01-mesh)
gvk_add_sample(getting-started-02-uniform-buffer)
gvk_add_sample(getting-started-03-texture-mapping)
gvk_add_sample(getting-started-04-render-target)
