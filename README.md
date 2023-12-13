
# Intel® Graphics Performance Analyzer Utilities for Vulkan*

A collection of Vulkan C++ utilities with a general focus on tools development, and a specific focus on supporting [Intel Graphics Performance Analyzers Framework](https://intel.github.io/gpasdk-doc/).

Features:
 - Vulkan structure utilities (compare/copy/serialize/stringify)
 - Managed Vulkan handles
 - Managed WSI (Window System Integration)
 - [ImGui](https://github.com/ocornut/imgui) integration
 - SPIR-V compilation via [glslang](https://github.com/KhronosGroup/glslang)
 - SPIR-V reflection via [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross)
 - [Vulkan Memory Allocator](https://gpuopen.com/vulkan-memory-allocator/) integration
 - Vulkan XML parsing utilities (used to keep the project up to date with the vk.xml)
 - ...and more...

# Getting Started
Ensure the following tools are installed...
 - [CMake](https://cmake.org/download/) v3.3+ (Make sure to select "Add to PATH" when prompted)
 - [Git](https://git-scm.com/)
 - [Python](https://www.python.org/downloads/) v3+ (Make sure to select "Add to PATH" when prompted)
 - [Visual Studio](https://visualstudio.microsoft.com/vs/community/) 2022 (Make sure to select "Desktop development with C++" when prompted)
 - [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) v1.3.268.0

The following command lines are for configuring a Visual Studio solution using a  `bash` like terminal (Git Bash comes with the Git install by default on Windows) in a directory called `gitrepos/intel` on drive `C:`...
```
cd c:
cd gitrepos/intel
git clone https://github.com/intel/gvk.git
cd gvk/
mkdir build
cd build/
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build .
```
...open `gvk/build/gvk.sln` in Visual Studio, navigate to `gvk/samples/getting-started-00-triangle`, right click and select "Set as Startup Project", run.

For Linux, replace `cmake -G "Visual Studio 17 2022" -A x64 ..` with `cmake ..` for the default Makefile generator.  See CMake's documentation for other generators.  Note that Windows support is further along than Linux, ymmv.

# External use
Somewhere in your CMakeLists, add the following...
```
include(FetchContent)
set(GVK_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GVK_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    gvk
    GIT_REPOSITORY "https://github.com/intel/gvk.git"
    GIT_TAG <desired commit hash>
)
FetchContent_MakeAvailable(gvk)
```
...then...
```
target_link_libraries(someTarget PUBLIC gvk)
```
...or link individual modules...
```
target_link_libraries(someTarget PUBLIC gvk-handles gvk-xml)
```
