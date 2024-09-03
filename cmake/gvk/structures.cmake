
include_guard(GLOBAL)
gvk_enable_target(gvk-structures)

include(gvk/cppgen)
include(gvk/runtime)
include(external/cereal)

add_subdirectory(gvk-structures)
