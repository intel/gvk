
# TODO : (in no particular order)
   -  glTF save/load https://www.khronos.org/gltf/
   -  Android support
   -  HLSL support
   -  CMake functionality to add shader compilation to build (both for gvk itself and consuming libraries)
   -  Break direct dependency on GLFW (so gvk::system::Surface can be created from an SDL_Window, for instance)
   -  Create info union for handles
   -  Layer tests need to depend on their layer's build so the layer is present on the first run of the test
   -  Generators can use some refactoring/cleanup
   -  Manual structure serialization/stringification/comparisons need to be filled out
   -  Multidevice validation...ie. run all tests that create a Vulkan context on every device available on the system
   -  Get rid of VkResult member in command structures
   -  It would be better to not have all Images carry around SwapchainKHR info since the vast majority of Images don't need it
   -  VK_EXT_layer_settings
   -  gvk/cmake/SPIRV-Cross.cmake, removed `set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE)`...this (and other dependency options) should be configurable
   -  Use std overloads that take error code parameters (ie. noexcept) where appropriate
   -  Multibuffer frame resources for samples
   -  All refernce types should set references only after successful creation so that in the case of failures the passed in parameter is untouched
   -  Mutliqueue support for gvk::RenderTarget and gvk::wsi::Context
   -  Seems ALL_BUILD should depend on tests.  Currently making a change in test code doesn't trigger a retest after rebuild.
   -  Refactor pNext generators to use the type-erased-structure-<functionality>-generator utilities.
   -  gvk.build.cmake installation procedures need some DRY after setting up all the `internal` install stuff

   -  VkVideo has been promoted to KHR, get codegen setup for these structures
      -  If/when supporting VkVideo, gpa-framework/src/playback/playback/source/token-categories.cpp needs update

   -  Better VkDescriptorSet state tracker tests
      -  Very complex, need to find a good strategy for procedurally testing descriptor logic (some kind of fuzz test might be a good solution?), it would be error prone and time consuming to try to write good test cases by hand

   -  Remove Auto<> from handles and use create/destroy_structure_copy() directly
      -  Plumb through VkAllocationCallbacks
      -  Get VkAllocationCallbacks from parent handle when necessary
      -  Set VkSystemAllocationScope appropriately

   -  Tests can use some refactoring/cleanup
      -  Abstractions to make authoring tests easier, this should be exposed as a lib that can be used with any unit test framework (ie. it should report failures, not actually trigger googletest failures directly)
      -  Validation context to respond to output from validation layer

   -  Rework layer interfaces (ie. the header associated with the layer)
      -  Linking to the interface shouldn't also add the layer implementation include directory
      -  Interface should automatically create a CMake INTERFACE/STATIC library as appropriate
      -  Layer interface structures should be able to have codegen for stringification/serialization/comparsions/etc.
      -  Codegen for layer entrypoint loading
      -  Restore point layer interface really needs access to the state tracker interface

   -  RestoreInfo cleanup
      -  GvkSwapchainImageRestoreInfo
      -  GvkSwapchainRestoreInfoKHR
      -  GvkAccelerationStructureSerilizationInfoKHR
      -  GvkAccelerationStructureRestoreInfoKHR

   -  VK_LAYER_INTEL_gvk_virtual_swapchain
      -  Shouldn't need to link gvk-handles, gvk-handles utilities should be broken out to its own library
      -  Generate layer hooks that assert() unserviced entrypoints that reference VkSwapchainKHR
         -  gvk::xml::get_commands_referencing_type(manifest, "VkSwapchainKHR")

   -  VK_LAYER_INTEL_gvk_restore_point
      -  Optionally direct repeat restore point to stream directory
      -  When repeating, only donwload modified resources...direct memory?
      -  gvkDestroyRestorePoint() should optionally cleanup directory
      -  Straighten out logs
      -  Reduce repeat download

   -  VK_LAYER_INTEL_gvk_state_tracker
      -  ObjectTracker::enumerate() callback should return a bool

   -  gvk/gvk-restore-point/include/gvk-restore-point/layer.hpp
      - Needs to be hooked up to...
         - gpa-framework/src/playback/playback/source/vulkan-range-repeat-cache-cmdbuffer.cpp
         - gpa-framework/src/playback/playback/source/vulkan-range-repeat-cache-descriptors.cpp
         - gpa-framework/src/playback/playback/source/vulkan-range-repeat-cache-resources.cpp
         - gpa-framework/src/playback/playback/source/vulkan-range-repeat-cache-swapchain.cpp
         - gpa-framework/src/playback/playback/source/vulkan-range-repeat-cache-synchronization.cpp

   -  There's some assumptions made regarding vk.xml API elements...
      Example from gvk/gvk-xml/source/gvk-xml/parameter.cpp:
         if (length == "1") {
            // NOTE : vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI() parameter
            //  pMaxWorkgroupSize is the only API element that specifies a length of 1 for
            //  a pointer parameter...clearing the length ensures that downstream codegen
            //  doesn't get confused.  The assert() is here to catch if other API elements
            //  get this treatment...if so, may need to actually deal with it.
            assert(name == "pMaxWorkgroupSize");
            length.clear();
         }
      ...need to setup a vk.xml validator to catch if assumptions are invalidated.

Candidate 3D Mesh Manipulation Libraries
-   https://github.com/gcherchi/InteractiveAndRobustMeshBooleans
-   https://github.com/elalish/manifold
-   https://gitlab.vci.rwth-aachen.de:9000/OpenMesh/OpenMesh
-   https://github.com/Kitware/VTK
