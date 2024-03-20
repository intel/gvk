
################################################################################
# VK_LAYER_INTEL_gvk_restore_point.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-restore-point/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-restore-point/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/basic-applier.hpp"
    "${generatedIncludePath}/basic-creator.hpp"
    "${generatedIncludePath}/basic-layer.hpp"
    "${generatedIncludePath}/update-structure-handles.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/applier-process-command-buffer.cpp"
    "${generatedSourcePath}/basic-applier.cpp"
    "${generatedSourcePath}/basic-creator.cpp"
    "${generatedSourcePath}/basic-layer.cpp"
    "${generatedSourcePath}/creator-process-command-buffer.cpp"
    "${generatedSourcePath}/update-structure-handles.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    TARGET
        VK_LAYER_INTEL_gvk_restore_point.generator
    FOLDER
        "VK_LAYER_INTEL_gvk_restore_point/"
    LINK_LIBRARIES
        gvk-cppgen
    INCLUDE_DIRECTORIES
        "${generatorSourcePath}/"
    INCLUDE_FILES
        "${generatorSourcePath}/applier-process-command-buffer.generator.hpp"
        "${generatorSourcePath}/basic-applier.generator.hpp"
        "${generatorSourcePath}/basic-creator.generator.hpp"
        "${generatorSourcePath}/basic-layer.generator.hpp"
        "${generatorSourcePath}/creator-process-command-buffer.generator.hpp"
        "${generatorSourcePath}/update-structure-handles.generator.hpp"
    SOURCE_FILES
        "${generatorSourcePath}/main.cpp"
    OUTPUT_FILES
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    COMPILE_DEFINITIONS
        GVK_RESTORE_POINT_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_RESTORE_POINT_GENERATED_INCLUDE_PREFIX="gvk-restore-point/generated/"
        GVK_RESTORE_POINT_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)

################################################################################
# VK_LAYER_INTEL_gvk_restore_point
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-restore-point/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-restore-point/")
gvk_add_layer(
    TARGET
        VK_LAYER_INTEL_gvk_restore_point
    FOLDER
        "VK_LAYER_INTEL_gvk_restore_point/"
    LINK_LIBRARIES
        gvk-command-structures
        gvk-format-info
        gvk-handles
        gvk-reference
        gvk-restore-info
        gvk-runtime
        VK_LAYER_INTEL_gvk_state_tracker-interface
        asio
        stb
        Threads::Threads
    INTERFACE_FILES
        "${includeDirectory}/VK_LAYER_INTEL_gvk_restore_point.h"
        "${includeDirectory}/VK_LAYER_INTEL_gvk_restore_point.hpp"
    INCLUDE_DIRECTORIES
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    INCLUDE_FILES
        "${generatedIncludeFiles}"
        "${includePath}/applier.hpp"
        "${includePath}/copy-engine.hpp"
        "${includePath}/creator.hpp"
        "${includePath}/layer.hpp"
        "${includePath}/logger.hpp"
        "${includePath}/utilities.hpp"
    SOURCE_FILES
        "${generatedSourceFiles}"
        "${sourcePath}/handles/acceleration-structure.cpp"
        "${sourcePath}/handles/buffer.cpp"
        "${sourcePath}/handles/command-buffer.cpp"
        "${sourcePath}/handles/descriptor-set.cpp"
        "${sourcePath}/handles/device.cpp"
        "${sourcePath}/handles/device-memory.cpp"
        "${sourcePath}/handles/event.cpp"
        "${sourcePath}/handles/fence.cpp"
        "${sourcePath}/handles/image.cpp"
        "${sourcePath}/handles/instance.cpp"
        "${sourcePath}/handles/pipeline.cpp"
        "${sourcePath}/handles/physical-device.cpp"
        "${sourcePath}/handles/query-pool.cpp"
        "${sourcePath}/handles/queue.cpp"
        "${sourcePath}/handles/semaphore.cpp"
        "${sourcePath}/handles/surface.cpp"
        "${sourcePath}/handles/swapchain.cpp"
        "${sourcePath}/applier.cpp"
        "${sourcePath}/copy-engine.cpp"
        "${sourcePath}/creator.cpp"
        "${sourcePath}/layer.cpp"
        "${sourcePath}/logger.cpp"
        "${sourcePath}/utilities.cpp"
    DESCRIPTION
        "Intel(R) GPA Utilities for Vulkan* restore point"
    ENTRY_POINTS
        gvkCreateRestorePoint
        gvkGetRestorePointObjects
        gvkDestroyRestorePoint
        gvkApplyRestorePoint
)
if(MSVC)
    set_source_files_properties("${generatedSourcePath}/update-structure-handles.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
endif()

################################################################################
# VK_LAYER_INTEL_gvk_restore_point install
gvk_install_library(TARGET VK_LAYER_INTEL_gvk_restore_point-interface)
gvk_install_layer(TARGET VK_LAYER_INTEL_gvk_restore_point)
