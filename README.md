
# Intel® Graphics Performance Analyzer Utilities for Vulkan*

A collection of Vulkan C++ utilities with a general focus on tools development, and a specific focus on supporting [Intel Graphics Performance Analyzers Framework](https://intel.github.io/gpasdk-doc/).

Features:
 - Comparison operators for Vulkan structures
 - Auto copy/destroy Vulkan structures
 - Stringify Vulkan structures
 - Serialize/deserialize Vulkan structures
 - Managed Vulkan handles
 - Managed WSI (Window System Integration)
 - Managed Mesh
 - Managed RenderTarget
 - SPIR-V compilation via [glslang](https://github.com/KhronosGroup/glslang)
 - SPIR-V reflection via [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross)
 - [Vulkan Memory Allocator](https://gpuopen.com/vulkan-memory-allocator/) integration
 - Vulkan XML parsing utilities (used to keep the project up to date with the vk.xml)

# Getting Started

NOTE : Currently only Windows only, this limitation will be lifted very soon

Ensure the following tools are installed...
 - [CMake](https://cmake.org/download/) v3.3+ (Make sure to select "Add to PATH" when prompted)
 - [Git](https://git-scm.com/)
 - [Python](https://www.python.org/downloads/) v3+ (Make sure to select "Add to PATH" when prompted)
 - [Visual Studio](https://visualstudio.microsoft.com/vs/community/) 2019 (Make sure to select "Desktop development with C++" when prompted)
 - [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) v1.3.216.0

The following instructions are for a  `bash` like terminal (Git Bash comes with the Git install by default on Windows)...
```
cd c:
cd <desired/directory/location/>
git clone https://github.com/intel/gvk.git
cd gvk/
mkdir build
cd build/
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build .
```
On Windows, open `gvk/build/gvk.sln` in Visual Studio, navigate to `gvk/samples/getting-started-00-triangle`, right click and select "Set as Startup Project", run.

# External use
Somewhere in your CMakeLists, add the following...
```
include(FetchContent)
set(GVK_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GVK_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    gvk
    GIT_REPOSITORY "https://github.com/intel/gvk.git"
    GIT_TAG <desired commit hash/tag>
)
FetchContent_MakeAvailable(gvk)
```
...then...
```
target_link_libraries(someTarget PUBLIC gvk)
```

# TODO : (in no particular order)
 - BuildOptions.cmake (enable/disable/customize dependencies/tests/samples/etc)
 - VkLayer_INTEL_gvk_state_tracker
 - VkLayer_INTEL_gvk_restore_point
 - glTF save/load https://www.khronos.org/gltf/
 - imgui integration https://github.com/ocornut/imgui
 - Android support
 - Linux support
 - HLSL support
 - CMake functionality to add shader compilation to build (both for gvk itself and consuming libraries)
 - Break direct dependency on GLFW (so gvk::sys::Surface can be created from an SDL_Window, for instance)
 - ...and of course, always new Vulkan features/extensions to stay on top of...
