
################################################################################
# gvk-restore-point.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-restore-point/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-restore-point/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/basic-restore-point-applier.hpp"
    "${generatedIncludePath}/basic-restore-point-creator.hpp"
    "${generatedIncludePath}/restore-info.h"
    "${generatedIncludePath}/restore-info-enumerations-to-string.hpp"
    "${generatedIncludePath}/restore-info-structure-cerealization.hpp"
    "${generatedIncludePath}/restore-info-structure-comparison-operators.hpp"
    "${generatedIncludePath}/restore-info-structure-create-copy.hpp"
    "${generatedIncludePath}/restore-info-structure-decerealization.hpp"
    "${generatedIncludePath}/restore-info-structure-deserialization.hpp"
    "${generatedIncludePath}/restore-info-structure-destroy-copy.hpp"
    "${generatedIncludePath}/restore-info-structure-get-stype.hpp"
    "${generatedIncludePath}/restore-info-structure-make-tuple.hpp"
    "${generatedIncludePath}/restore-info-structure-serialization.hpp"
    "${generatedIncludePath}/restore-info-structure-to-string.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/apply-command-buffer-restore-point.cpp"
    "${generatedSourcePath}/basic-restore-point-applier.cpp"
    "${generatedSourcePath}/basic-restore-point-creator.cpp"
    "${generatedSourcePath}/create-command-buffer-restore-point.cpp"
    "${generatedSourcePath}/restore-info-enumerations-to-string.cpp"
    "${generatedSourcePath}/restore-info-structure-comparison-operators.cpp"
    "${generatedSourcePath}/restore-info-structure-create-copy.cpp"
    "${generatedSourcePath}/restore-info-structure-deserialization.cpp"
    "${generatedSourcePath}/restore-info-structure-destroy-copy.cpp"
    "${generatedSourcePath}/restore-info-structure-serialization.cpp"
    "${generatedSourcePath}/restore-info-structure-to-string.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    TARGET
        gvk-restore-point.generator
    FOLDER
        "gvk-restore-point/"
    LINK_LIBRARIES
        gvk-cppgen
    INCLUDE_DIRECTORIES
        "${generatorSourcePath}/"
    INCLUDE_FILES
        "${generatorSourcePath}/apply-command-buffer-restore-point.generator.hpp"
        "${generatorSourcePath}/basic-restore-point-applier.generator.hpp"
        "${generatorSourcePath}/basic-restore-point-creator.generator.hpp"
        "${generatorSourcePath}/create-command-buffer-restore-point.generator.hpp"
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
# gvk-restore-point
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-restore-point/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-restore-point/")
set(linkLibraries "")
gvk_add_static_library(
    TARGET
        gvk-restore-point
    FOLDER
        "gvk-restore-point/"
    LINK_LIBRARIES
        gvk-command-structures
        gvk-handles
        gvk-reference
        gvk-runtime
        VK_LAYER_INTEL_gvk_state_tracker-interface
        asio
        Threads::Threads
    INCLUDE_DIRECTORIES
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    INCLUDE_FILES
        "${generatedIncludeFiles}"
        "${includePath}/detail/asio-include.hpp"
        "${includePath}/detail/copy-engine.hpp"
        "${includePath}/detail/restore-point-applier-base.hpp"
        "${includePath}/restore-point-applier.hpp"
        "${includePath}/restore-point-creator.hpp"
        "${includePath}/restore-point-info.hpp"
        "${includeDirectory}/gvk-restore-point.hpp"
    SOURCE_FILES
        "${generatedSourceFiles}"
        "${sourcePath}/detail/copy-engine.cpp"
        "${sourcePath}/detail/restore-info-to-string-manual.cpp"
        "${sourcePath}/restore-point-applier.cpp"
        "${sourcePath}/restore-point-creator.cpp"
)

################################################################################
# gvk-restore-point install
gvk_install_library(TARGET gvk-restore-point)
