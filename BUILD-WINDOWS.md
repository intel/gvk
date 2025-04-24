
--------------------------------------------------------------------------------
### Build Windows

#### Quick Start
- If your development environment is already setup
- Using GitBash/MinGW from the GVK root directory
> `time cmake -G "Visual Studio 17 2022" -A x64 -B build`  
> `time cmake --build build --target install`  
    - Note that the `time` command isn't necessary, but is added for conveneience
- Open the Visual Studio solution at `gvk/build/gvk.sln`
- Navigate to `gvk/samples/gvk-getting-started-00-triangle`
- Right click and select **Set as Startup Project**
- Run

#### Detailed Instructions
- [Install Build Tools](#Install-Build-Tools)
- [Clone Repository](#Clone-Repository)
- [Configure and Build](#Configure-and-Build)
- [Configure Vulkan SDK](#Configure-Vulkan-SDK)

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
## Detailed Instructions
--------------------------------------------------------------------------------
### Install Build Tools
- Install [CMake](https://cmake.org/download/) (Make sure to select "Add to PATH" when prompted)
- Install [Git](https://git-scm.com/)
- Install [Python](https://www.python.org/downloads/) (Make sure to select "Add to PATH" when prompted)
- Install [Visual Studio](https://visualstudio.microsoft.com/vs/community/) (Make sure to select "Desktop development with C++" when prompted)
- Install [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) (optional; see the [Configure Vulkan SDK](#Configure-Vulkan-SDK) section for more info)

--------------------------------------------------------------------------------
### Clone Repository
- Using Git Bash/MinGW
- Create a "/c/intel" directory and clone GVK there, feel free to use a different directory if desired
> `mkdir /c/intel`  
> `cd /c/intel`  
> `git clone https://github.com/intel/gvk.git`

--------------------------------------------------------------------------------
### Configure and Build
- Using GitBash/MinGW from the GVK root directory
> `time cmake -G "Visual Studio 17 2022" -A x64 -B build`  
> `time cmake --build build --target install`  
    - Note that the `time` command isn't necessary, but is added for conveneience
- Open the Visual Studio solution at `gvk/build/gvk.sln`
- Navigate to `gvk/samples/gvk-getting-started-00-triangle`
- Right click and select **Set as Startup Project**
- Run

--------------------------------------------------------------------------------
### Configure Vulkan SDK
- GVK will download the required Vulkan SDK at configure time if necessary; see GVK's root CMakeLists for the required version
- The GVK build does not install the Vulkan SDK, set environment variables, system path, or Windows Vulkan layer registry entries
- To set these after GVK configuration, the Vulkan SDK installer can be run from `gvk/build/_deps/VulkanSDK/`
    - Make sure to close/reopen terminals (or otherwise refresh environment variables) after installation
- Alternatively, the downloaded Vulkan SDK can be used directly by setting the environment variable `VULKAN_SDK` to point to `gvk/build/_deps/VulkanSDK/<version>/`
    - For more info on configuring layers see
        - https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderLayerInterface.md
- For advanced users configuring automated environments the following scripts are provided
    - For Windows registry configuration
        - `gvk/gvk-runtime/gvk-windows-layer-registry.ps1`
        - `gvk/install/bin/<config>/gvk-windows-layer-registry.ps1`
    - For downloading and extracting the required Vulkan SDK
        - `gvk/build/cmake/gvk-vulkan-sdk.cmake`
        - `gvk/install/cmake/gvk-vulkan-sdk.cmake`
