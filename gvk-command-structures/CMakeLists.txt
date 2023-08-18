
################################################################################
# gvk-command-structures.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-command-structures/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-command-structures/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/command.h"
    "${generatedIncludePath}/command-enumerations-to-string.hpp"
    "${generatedIncludePath}/command-structure-cerealization.hpp"
    "${generatedIncludePath}/command-structure-comparison-operators.hpp"
    "${generatedIncludePath}/command-structure-create-copy.hpp"
    "${generatedIncludePath}/command-structure-decerealization.hpp"
    "${generatedIncludePath}/command-structure-deserialization.hpp"
    "${generatedIncludePath}/command-structure-destroy-copy.hpp"
    "${generatedIncludePath}/command-structure-enumerate-handles.hpp"
    "${generatedIncludePath}/command-structure-get-stype.hpp"
    "${generatedIncludePath}/command-structure-make-tuple.hpp"
    "${generatedIncludePath}/command-structure-serialization.hpp"
    "${generatedIncludePath}/command-structure-to-string.hpp"
    "${generatedIncludePath}/execute-command-structure.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/command-enumerations-to-string.cpp"
    "${generatedSourcePath}/command-structure-comparison-operators.cpp"
    "${generatedSourcePath}/command-structure-create-copy.cpp"
    "${generatedSourcePath}/command-structure-deserialization.cpp"
    "${generatedSourcePath}/command-structure-destroy-copy.cpp"
    "${generatedSourcePath}/command-structure-enumerate-handles.cpp"
    "${generatedSourcePath}/command-structure-serialization.cpp"
    "${generatedSourcePath}/command-structure-to-string.cpp"
    "${generatedSourcePath}/execute-command-structure.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    TARGET
        gvk-command-structures.generator
    FOLDER
        "gvk-command-structures/"
    LINK_LIBRARIES
        gvk-cppgen
    INCLUDE_FILES
        "${generatorSourcePath}/execute-command-structure.generator.hpp"
    SOURCE_FILES
        "${generatorSourcePath}/main.cpp"
    OUTPUT_FILES
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    COMPILE_DEFINITIONS
        GVK_COMMAND_STRUCTURES_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_COMMAND_STRUCTURES_GENERATED_INCLUDE_PREFIX="gvk-command-structures/generated/"
        GVK_COMMAND_STRUCTURES_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)

################################################################################
# gvk-command-structures
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-command-structures/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-command-structures/")
gvk_add_static_library(
    TARGET
        gvk-command-structures
    FOLDER
        "gvk-command-structures/"
    LINK_LIBRARIES
        gvk-runtime
        gvk-structures
    INCLUDE_DIRECTORIES
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    INCLUDE_FILES
        "${generatedIncludeFiles}"
        "${includePath}/detail/cerealization-manual.hpp"
        "${includePath}/detail/make-tuple-manual.hpp"
        "${includeDirectory}/gvk-command-structures.hpp"
    SOURCE_FILES
        "${generatedSourceFiles}"
        "${sourcePath}/detail/copy-manual.cpp"
        "${sourcePath}/detail/handle-enumeration-manual.cpp"
        "${sourcePath}/detail/to-string-manual.cpp"
)
if(MSVC)
    set_source_files_properties("${generatedSourcePath}/command-structure-comparison-operators.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
endif()

################################################################################
# gvk-command-structures install
gvk_install_library(TARGET gvk-command-structures)
