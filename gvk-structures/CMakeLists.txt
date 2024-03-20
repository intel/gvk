
################################################################################
# gvk-structures.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-structures/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-structures/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/cerealize-pnext.hpp"
    "${generatedIncludePath}/core-enumerations-to-string.hpp"
    "${generatedIncludePath}/core-structure-cerealization.hpp"
    "${generatedIncludePath}/core-structure-comparison-operators.hpp"
    "${generatedIncludePath}/core-structure-create-copy.hpp"
    "${generatedIncludePath}/core-structure-decerealization.hpp"
    "${generatedIncludePath}/core-structure-deserialization.hpp"
    "${generatedIncludePath}/core-structure-destroy-copy.hpp"
    "${generatedIncludePath}/core-structure-enumerate-handles.hpp"
    "${generatedIncludePath}/core-structure-get-stype.hpp"
    "${generatedIncludePath}/core-structure-make-tuple.hpp"
    "${generatedIncludePath}/core-structure-serialization.hpp"
    "${generatedIncludePath}/core-structure-to-string.hpp"
    "${generatedIncludePath}/decerealize-pnext.hpp"
    "${generatedIncludePath}/get-object-type.hpp"
    "${generatedIncludePath}/handle-to-string.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/create-pnext-copy.cpp"
    "${generatedSourcePath}/core-enumerations-to-string.cpp"
    "${generatedSourcePath}/core-structure-comparison-operators.cpp"
    "${generatedSourcePath}/core-structure-create-copy.cpp"
    "${generatedSourcePath}/core-structure-deserialization.cpp"
    "${generatedSourcePath}/core-structure-destroy-copy.cpp"
    "${generatedSourcePath}/core-structure-enumerate-handles.cpp"
    "${generatedSourcePath}/core-structure-serialization.cpp"
    "${generatedSourcePath}/core-structure-to-string.cpp"
    "${generatedSourcePath}/destroy-pnext-copy.cpp"
    "${generatedSourcePath}/enumerate-pnext-handles.cpp"
    "${generatedSourcePath}/handle-to-string.cpp"
    "${generatedSourcePath}/pnext-to-string.cpp"
    "${generatedSourcePath}/pnext-tuple-element-wrapper.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    TARGET
        gvk-structures.generator
    FOLDER
        "gvk-structures/"
    LINK_LIBRARIES
        gvk-cppgen
    INCLUDE_DIRECTORIES
        "${generatorSourcePath}/"
    INCLUDE_FILES
        "${generatorSourcePath}/cerealize-pnext.generator.hpp"
        "${generatorSourcePath}/create-pnext-copy.generator.hpp"
        "${generatorSourcePath}/decerealize-pnext.generator.hpp"
        "${generatorSourcePath}/destroy-pnext-copy.generator.hpp"
        "${generatorSourcePath}/enumerate-pnext-handles.generator.hpp"
        "${generatorSourcePath}/get-object-type.generator.hpp"
        "${generatorSourcePath}/handle-to-string.generator.hpp"
        "${generatorSourcePath}/pnext-to-string.generator.hpp"
        "${generatorSourcePath}/pnext-tuple-element-wrapper.generator.hpp"
    SOURCE_FILES
        "${generatorSourcePath}/main.cpp"
    OUTPUT_FILES
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    COMPILE_DEFINITIONS
        GVK_STRUCTURES_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_STRUCTURES_GENERATED_INCLUDE_PREFIX="gvk-structures/generated/"
        GVK_STRUCTURES_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)
if(MSVC)
    set_source_files_properties("${generatedSourcePath}/core-structure-comparison-operators.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
endif()

################################################################################
# gvk-structures
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-structures/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-structures/")
gvk_add_static_library(
    TARGET
        gvk-structures
    FOLDER
        "gvk-structures/"
    LINK_LIBRARIES
        gvk-runtime
        cereal
        # cereal_INTERFACE
    INCLUDE_DIRECTORIES
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
        # "${cerealSourceDirectory}/include/"
    INCLUDE_FILES
        "${generatedIncludeFiles}"
        "${includePath}/detail/cerealization-manual.hpp"
        "${includePath}/detail/cerealization-utilities.hpp"
        "${includePath}/detail/get-count.hpp"
        "${includePath}/detail/get-stype-utilities.hpp"
        "${includePath}/detail/handle-enumeration-utilities.hpp"
        "${includePath}/detail/make-tuple-manual.hpp"
        "${includePath}/detail/make-tuple-utilities.hpp"
        "${includePath}/detail/copy-utilities.hpp"
        "${includePath}/detail/to-string-utilities.hpp"
        "${includePath}/auto.hpp"
        "${includePath}/comparison-operators.hpp"
        "${includePath}/convert.hpp"
        "${includePath}/copy.hpp"
        "${includePath}/defaults.hpp"
        "${includePath}/enumerate-handles.hpp"
        "${includePath}/get-object-type.hpp"
        "${includePath}/get-stype.hpp"
        "${includePath}/pnext.hpp"
        "${includePath}/serialization.hpp"
        "${includePath}/to-string.hpp"
        "${includeDirectory}/gvk-structures.hpp"
    SOURCE_FILES
        "${generatedSourceFiles}"
        "${sourcePath}/detail/cerealization-utilities.cpp"
        "${sourcePath}/detail/copy-manual.cpp"
        "${sourcePath}/detail/handle-enumeration-manual.cpp"
        "${sourcePath}/detail/make-tuple-utilities.cpp"
        "${sourcePath}/detail/to-string-manual.cpp"
        "${sourcePath}/defaults.cpp"
)

################################################################################
# gvk-structures.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    TARGET
        gvk-structures
    FOLDER
        "gvk-structures/"
    SOURCE_FILES
        "${testsPath}/comparison-operator.tests.cpp"
        "${testsPath}/copy.tests.cpp"
        "${testsPath}/handle-enumeration.tests.cpp"
        "${testsPath}/serialization.tests.cpp"
        "${testsPath}/to-string.tests.cpp"
        "${testsPath}/validate-structure-serialization.hpp"
        "${testsPath}/validation-allocator.hpp"
)

################################################################################
# gvk-structures install
gvk_install_library(TARGET gvk-structures)
