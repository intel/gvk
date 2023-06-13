
################################################################################
# VK_LAYER_INTEL_gvk_state_tracker.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-state-tracker/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-state-tracker/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/basic-state-tracker.hpp"
    "${generatedIncludePath}/basic-cmd-tracker.hpp"
    "${generatedIncludePath}/forward-declarations.inl"
    "${generatedIncludePath}/state-tracked-handles.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/basic-state-tracker.cpp"
    "${generatedSourcePath}/basic-cmd-tracker.cpp"
    "${generatedSourcePath}/enumerate-state-tracked-objects.cpp"
    "${generatedSourcePath}/get-state-tracked-object-create-info.cpp"
    "${generatedSourcePath}/get-state-tracked-object-info.cpp"
    "${generatedSourcePath}/state-tracked-handles.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    target
        VK_LAYER_INTEL_gvk_state_tracker.generator
    folder
        "gvk-state-tracker/"
    linkLibraries
        gvk-cppgen
    includeDirectories
        "${generatorSourcePath}/"
    includeFiles
        "${generatorSourcePath}/basic-state-tracker.generator.hpp"
        "${generatorSourcePath}/basic-cmd-tracker.generator.hpp"
        "${generatorSourcePath}/enumerate-state-tracked-objects.generator.hpp"
        "${generatorSourcePath}/get-state-tracked-object-create-info.generator.hpp"
        "${generatorSourcePath}/get-state-tracked-object-info.generator.hpp"
        "${generatorSourcePath}/state-tracked-handles.generator.hpp"
    sourceFiles
        "${generatorSourcePath}/main.cpp"
    outputFiles
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    compileDefinitions
        GVK_STATE_TRACKER_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_STATE_TRACKER_GENERATED_INCLUDE_PREFIX="gvk-state-tracker/generated/"
        GVK_STATE_TRACKER_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)

################################################################################
# VK_LAYER_INTEL_gvk_state_tracker
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-state-tracker/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-state-tracker/")
gvk_add_layer(
    target
        VK_LAYER_INTEL_gvk_state_tracker
    folder
        "gvk-state-tracker/"
    linkLibraries
        gvk-command-structures
        gvk-reference
        gvk-runtime
    interfaceFiles
        "${includeDirectory}/VK_LAYER_INTEL_gvk_state_tracker.h"
        "${includeDirectory}/VK_LAYER_INTEL_gvk_state_tracker.hpp"
    includeDirectories
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    includeFiles
        "${generatedIncludeFiles}"
        "${includePath}/cmd-tracker.hpp"
        "${includePath}/dependency-enumerator.hpp"
        "${includePath}/descriptor.hpp"
        "${includePath}/image-layout-tracker.hpp"
        "${includePath}/memory-map-info.hpp"
        "${includePath}/object-tracker.hpp"
        "${includePath}/state-tracker.hpp"
        "${includePath}/thread-safe-unordered-map.hpp"
    sourceFiles
        "${generatedSourceFiles}"
        "${sourcePath}/buffer.cpp"
        "${sourcePath}/cmd-tracker.cpp"
        "${sourcePath}/command-buffer.cpp"
        "${sourcePath}/dependency-enumerator.cpp"
        "${sourcePath}/descriptor-set.cpp"
        "${sourcePath}/device-memory.cpp"
        "${sourcePath}/device.cpp"
        "${sourcePath}/framebuffer.cpp"
        "${sourcePath}/image-layout-tracker.cpp"
        "${sourcePath}/image.cpp"
        "${sourcePath}/instance.cpp"
        "${sourcePath}/pipeline.cpp"
        "${sourcePath}/shader.cpp"
        "${sourcePath}/state-tracked-handle-utilities.cpp"
        "${sourcePath}/state-tracker.cpp"
        "${sourcePath}/swapchain.cpp"
    description
        "Intel(R) GPA Utilities for Vulkan* state tracker"
    entryPoints
        gvkSetStateTrackerPhysicalDevices
        gvkEnumerateStateTrackedObjects
        gvkEnumerateStateTrackedObjectDependencies
        gvkEnumerateStateTrackedObjectBindings
        gvkEnumerateStateTrackedCommandBufferCmds
        gvkGetStateTrackedObjectInfo
        gvkGetStateTrackedObjectCreateInfo
        gvkGetStateTrackedObjectAllocateInfo
        gvkGetStateTrackedImageLayouts
        gvkGetStateTrackedMappedMemory
        gvkDisableStateTracker
        gvkEnableStateTracker
)
if(MSVC)
    set_source_files_properties("${generatedSourcePath}/basic-state-tracker.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
    set_source_files_properties("${generatedSourcePath}/state-tracked-handles.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
endif()

################################################################################
# VK_LAYER_INTEL_gvk_state_tracker.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    target
        VK_LAYER_INTEL_gvk_state_tracker
    folder
        "gvk-state-tracker/"
    linkLibraries
        gvk-handles
        gvk-math
        gvk-spirv
        gvk-system
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${testsPath}/state-tracker-test-utilities.hpp"
    sourceFiles
        "${testsPath}/command-buffer.tests.cpp"
        "${testsPath}/descriptor-set.tests.cpp"
        "${testsPath}/device-memory-binding.tests.cpp"
        "${testsPath}/image-layout.tests.cpp"
        "${testsPath}/pipeline.tests.cpp"
        "${testsPath}/state-tracker-test-utilities.cpp"
        "${testsPath}/swapchain.tests.cpp"
    compileDefinitions
        GVK_STATE_TRACKER_LAYER_JSON_PATH="$<TARGET_FILE_DIR:VK_LAYER_INTEL_gvk_state_tracker>"
)
