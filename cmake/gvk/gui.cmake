
include_guard(GLOBAL)
gvk_enable_target(gvk-gui)

include(gvk/handles)
include(gvk/spirv)
include(gvk/system)
include(external/imgui)

add_subdirectory(gvk-gui)
