
include_guard(GLOBAL)
gvk_enable_target(gvk-layer)

include(gvk/cppgen)
include(gvk/runtime)
include(gvk/structures)

add_subdirectory(gvk-layer)
