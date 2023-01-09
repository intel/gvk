
################################################################################
# gvk-structures.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-structures/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-structures/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/cerealize-pnext.hpp"
    "${generatedIncludePath}/command-structures.h"
    "${generatedIncludePath}/command-structures-cerealization.hpp"
    "${generatedIncludePath}/command-structures-comparison-operators.hpp"
    "${generatedIncludePath}/command-structures-create-copy.hpp"
    "${generatedIncludePath}/command-structures-decerealization.hpp"
    "${generatedIncludePath}/command-structures-deserialization.hpp"
    "${generatedIncludePath}/command-structures-destroy-copy.hpp"
    "${generatedIncludePath}/command-structures-get-stype.hpp"
    "${generatedIncludePath}/command-structures-make-tuple.hpp"
    "${generatedIncludePath}/command-structures-serialization.hpp"
    "${generatedIncludePath}/command-structures-to-string.hpp"
    "${generatedIncludePath}/command-structure-type-to-string.hpp"
    "${generatedIncludePath}/core-enumerations-to-string.hpp"
    "${generatedIncludePath}/core-structures-cerealization.hpp"
    "${generatedIncludePath}/core-structures-comparison-operators.hpp"
    "${generatedIncludePath}/core-structures-create-copy.hpp"
    "${generatedIncludePath}/core-structures-decerealization.hpp"
    "${generatedIncludePath}/core-structures-deserialization.hpp"
    "${generatedIncludePath}/core-structures-destroy-copy.hpp"
    "${generatedIncludePath}/core-structures-get-stype.hpp"
    "${generatedIncludePath}/core-structures-make-tuple.hpp"
    "${generatedIncludePath}/core-structures-serialization.hpp"
    "${generatedIncludePath}/core-structures-to-string.hpp"
    "${generatedIncludePath}/decerealize-pnext.hpp"
    "${generatedIncludePath}/handle-to-string.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/command-structures-comparison-operators.cpp"
    "${generatedSourcePath}/command-structures-create-copy.cpp"
    "${generatedSourcePath}/command-structures-deserialization.cpp"
    "${generatedSourcePath}/command-structures-destroy-copy.cpp"
    "${generatedSourcePath}/command-structures-serialization.cpp"
    "${generatedSourcePath}/command-structures-to-string.cpp"
    "${generatedSourcePath}/command-structure-type-to-string.cpp"
    "${generatedSourcePath}/core-enumerations-to-string.cpp"
    "${generatedSourcePath}/core-structures-comparison-operators.cpp"
    "${generatedSourcePath}/core-structures-create-copy.cpp"
    "${generatedSourcePath}/core-structures-deserialization.cpp"
    "${generatedSourcePath}/core-structures-destroy-copy.cpp"
    "${generatedSourcePath}/core-structures-serialization.cpp"
    "${generatedSourcePath}/core-structures-to-string.cpp"
    "${generatedSourcePath}/create-pnext-copy.cpp"
    "${generatedSourcePath}/destroy-pnext-copy.cpp"
    "${generatedSourcePath}/handle-to-string.cpp"
    "${generatedSourcePath}/pnext-to-string.cpp"
    "${generatedSourcePath}/pnext-tuple-element-wrapper.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    target
        gvk-structures.generator
    folder
        "gvk-structures/"
    linkLibraries
        gvk-cppgen
    includeDirectories
        "${generatorSourcePath}/"
    includeFiles
        "${generatorSourcePath}/cerealize-pnext.generator.hpp"
        "${generatorSourcePath}/cerealize-structures.generator.hpp"
        "${generatorSourcePath}/command-structures.generator.hpp"
        "${generatorSourcePath}/comparison-operators.generator.hpp"
        "${generatorSourcePath}/create-pnext-copy.generator.hpp"
        "${generatorSourcePath}/create-structure-copy.generator.hpp"
        "${generatorSourcePath}/decerealize-pnext.generator.hpp"
        "${generatorSourcePath}/decerealize-structures.generator.hpp"
        "${generatorSourcePath}/deserialize-structures.generator.hpp"
        "${generatorSourcePath}/destroy-pnext-copy.generator.hpp"
        "${generatorSourcePath}/destroy-structure-copy.generator.hpp"
        "${generatorSourcePath}/enum-to-string.generator.hpp"
        "${generatorSourcePath}/get-stype.generator.hpp"
        "${generatorSourcePath}/handle-to-string.generator.hpp"
        "${generatorSourcePath}/make-tuple.generator.hpp"
        "${generatorSourcePath}/pnext-to-string.generator.hpp"
        "${generatorSourcePath}/pnext-tuple-element-wrapper.generator.hpp"
        "${generatorSourcePath}/serialize-structures.generator.hpp"
        "${generatorSourcePath}/structure-to-string.generator.hpp"
        "${generatorSourcePath}/structure-utilities.generator.hpp"
    sourceFiles
        "${generatorSourcePath}/main.cpp"
    outputFiles
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    compileDefinitions
        GVK_STRUCTURES_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_STRUCTURES_GENERATED_INCLUDE_PREFIX="gvk-structures/generated/"
        GVK_STRUCTURES_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)
if(MSVC)
    set_source_files_properties("${generatedSourcePath}/command-structures-comparison-operators.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
    set_source_files_properties("${generatedSourcePath}/core-structures-comparison-operators.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
endif()

################################################################################
# gvk-structures
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-structures/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-structures/")
gvk_add_static_library(
    target
        gvk-structures
    folder
        "gvk-structures/"
    linkLibraries
        gvk-string
        cereal
        Vulkan::Vulkan
    includeDirectories
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    includeFiles
        "${generatedIncludeFiles}"
        "${includePath}/detail/cerealization-manual.hpp"
        "${includePath}/detail/cerealization-utilities.hpp"
        "${includePath}/detail/get-count.hpp"
        "${includePath}/detail/get-stype-utilities.hpp"
        "${includePath}/detail/make-tuple-manual.hpp"
        "${includePath}/detail/make-tuple-utilities.hpp"
        "${includePath}/detail/copy-utilities.hpp"
        "${includePath}/detail/to-string-utilities.hpp"
        "${includePath}/auto.hpp"
        "${includePath}/command-structures.hpp"
        "${includePath}/comparison-operators.hpp"
        "${includePath}/copy.hpp"
        "${includePath}/defaults.hpp"
        "${includePath}/defines.hpp"
        "${includePath}/get-stype.hpp"
        "${includePath}/serialization.hpp"
        "${includePath}/to-string.hpp"
    sourceFiles
        "${generatedSourceFiles}"
        "${sourcePath}/detail/cerealization-utilities.cpp"
        "${sourcePath}/detail/copy-manual.cpp"
        "${sourcePath}/detail/make-tuple-utilities.cpp"
        "${sourcePath}/detail/to-string-manual.cpp"
        "${sourcePath}/defaults.cpp"
)

################################################################################
# gvk-structures.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    target
        gvk-structures
    folder
        "gvk-structures/"
    sourceFiles
        "${testsPath}/comparison-operator.tests.cpp"
        "${testsPath}/copy.tests.cpp"
        "${testsPath}/serialization.tests.cpp"
        "${testsPath}/to-string.tests.cpp"
)
