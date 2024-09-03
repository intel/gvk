
include_guard(GLOBAL)
gvk_enable_target(gvk-handles)

include(gvk/cppgen)
include(gvk/format-info)
include(gvk/reference)
include(gvk/runtime)
include(gvk/string)
include(gvk/structures)
include(external/VulkanMemoryAllocator)

add_subdirectory(gvk-handles)
