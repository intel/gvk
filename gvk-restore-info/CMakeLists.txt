
################################################################################
# gvk-restore-info.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-restore-info/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-restore-info/generated/")
set(generatedIncludeFiles
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
        gvk-restore-info.generator
    FOLDER
        "gvk-restore-info/"
    LINK_LIBRARIES
        gvk-cppgen
    SOURCE_FILES
        "${generatorSourcePath}/main.cpp"
    OUTPUT_FILES
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    COMPILE_DEFINITIONS
        GVK_RESTORE_INFO_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_RESTORE_INFO_GENERATED_INCLUDE_PREFIX="gvk-restore-info/generated/"
        GVK_RESTORE_INFO_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)

################################################################################
# gvk-restore-info
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-restore-info/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-restore-info/")
gvk_add_static_library(
    TARGET
        gvk-restore-info
    FOLDER
        "gvk-restore-info/"
    LINK_LIBRARIES
        gvk-runtime
        gvk-structures
    INCLUDE_DIRECTORIES
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    INCLUDE_FILES
        "${generatedIncludeFiles}"
        "${includeDirectory}/gvk-restore-info.hpp"
    SOURCE_FILES
        "${generatedSourceFiles}"
        "${sourcePath}/detail/to-string-manual.cpp"
)

################################################################################
# gvk-restore-info install
gvk_install_library(TARGET gvk-restore-info)
