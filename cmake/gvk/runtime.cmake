
include_guard(GLOBAL)
gvk_enable_target(gvk-runtime)

include(gvk/cppgen)
include(gvk/string)
include(external/Vulkan)

add_subdirectory(gvk-runtime)
