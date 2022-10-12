
################################################################################
# gvk.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/cerealize-structures.hpp"
    "${generatedIncludePath}/comparison-operators.hpp"
    "${generatedIncludePath}/create-structure-copy.hpp"
    "${generatedIncludePath}/decerealize-structures.hpp"
    "${generatedIncludePath}/deserialize-structures.hpp"
    "${generatedIncludePath}/destroy-structure-copy.hpp"
    "${generatedIncludePath}/dispatch-table.hpp"
    "${generatedIncludePath}/enum-to-string.hpp"
    "${generatedIncludePath}/format-utilities.hpp"
    "${generatedIncludePath}/forward-declarations.hpp"
    "${generatedIncludePath}/get-stype.hpp"
    "${generatedIncludePath}/handle-to-string.hpp"
    "${generatedIncludePath}/handles.hpp"
    "${generatedIncludePath}/make-tuple.hpp"
    "${generatedIncludePath}/serialize-structures.hpp"
    "${generatedIncludePath}/structure-to-string.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/comparison-operators.cpp"
    "${generatedSourcePath}/create-structure-copy.cpp"
    "${generatedSourcePath}/deserialize-structures.cpp"
    "${generatedSourcePath}/destroy-structure-copy.cpp"
    "${generatedSourcePath}/dispatch-table.cpp"
    "${generatedSourcePath}/enum-to-string.cpp"
    "${generatedSourcePath}/format-utilities.cpp"
    "${generatedSourcePath}/handle-to-string.cpp"
    "${generatedSourcePath}/handles.cpp"
    "${generatedSourcePath}/make-tuple.cpp"
    "${generatedSourcePath}/serialize-structures.cpp"
    "${generatedSourcePath}/structure-to-string.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    target
        gvk.generator
    folder
        "tools/"
    linkLibraries
        gvk-string
        gvk-xml
    includeDirectories
        "${generatorSourcePath}/"
    includeFiles
        "${generatorSourcePath}/comparison-operators.generator.hpp"
        "${generatorSourcePath}/cppgen-utilities.hpp"
        "${generatorSourcePath}/cerealize-structures.generator.hpp"
        "${generatorSourcePath}/create-structure-copy.generator.hpp"
        "${generatorSourcePath}/decerealize-structures.generator.hpp"
        "${generatorSourcePath}/deserialize-structures.generator.hpp"
        "${generatorSourcePath}/destroy-structure-copy.generator.hpp"
        "${generatorSourcePath}/dispatch-table.generator.hpp"
        "${generatorSourcePath}/enum-to-string.generator.hpp"
        "${generatorSourcePath}/format-utilities.generator.hpp"
        "${generatorSourcePath}/forward-declarations.generator.hpp"
        "${generatorSourcePath}/get-stype.generator.hpp"
        "${generatorSourcePath}/handle-to-string.generator.hpp"
        "${generatorSourcePath}/handles.generator.hpp"
        "${generatorSourcePath}/make-tuple.generator.hpp"
        "${generatorSourcePath}/serialize-structures.generator.hpp"
        "${generatorSourcePath}/structure-to-string.generator.hpp"
    sourceFiles
        "${generatorSourcePath}/main.cpp"
    outputFiles
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    compileDefinitions
        GVK_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_GENERATED_INCLUDE_PREFIX="gvk/generated/"
        GVK_GENERATED_SOURCE_PATH="${generatedSourcePath}"
)

################################################################################
# gvk
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk/")
list(APPEND linkLibraries
    gvk-math
    gvk-string
    gvk-system
    gvk-xml
    Vulkan::Vulkan
)
if(GVK_ASIO_ENABLED)
    list(APPEND linkLibraries asio_INTERFACE)
endif()
if(GVK_CEREAL_ENABLED)
    list(APPEND linkLibraries cereal)
endif()
if(GVK_GLSLANG_ENABLED)
    list(APPEND linkLibraries glslang_INTERFACE)
endif()
if(GVK_SPIRV_CROSS_ENABLED)
    list(APPEND linkLibraries SPIRV-Cross_INTERFACE)
endif()
if(GVK_STB_ENABLED)
    list(APPEND linkLibraries stb)
endif()
if(GVK_VMA_ENABLED)
    list(APPEND linkLibraries VulkanMemoryAllocator)
endif()
gvk_add_static_library(
    target
        gvk
    linkLibraries
        ${linkLibraries}
    includeDirectories
        "${includeDirectory}"
        "${generatedIncludeDirectory}"
    includeFiles
        "${includePath}/detail/cerealization-utilities.hpp"
        "${includePath}/detail/comparison-operator-utilities.hpp"
        "${includePath}/detail/handle-utilities.hpp"
        "${includePath}/detail/make-tuple-utilities.hpp"
        "${includePath}/detail/reference.hpp"
        "${includePath}/detail/structure-copy-utilities.hpp"
        "${includePath}/detail/to-string-utilities.hpp"
        "${includePath}/context.hpp"
        "${includePath}/defaults.hpp"
        "${includePath}/defines.hpp"
        "${includePath}/format.hpp"
        "${includePath}/handles.hpp"
        "${includePath}/mesh.hpp"
        "${includePath}/render-target.hpp"
        "${includePath}/serialization.hpp"
        "${includePath}/spir-v.hpp"
        "${includePath}/structures.hpp"
        "${includePath}/to-string.hpp"
        "${includePath}/wsi-manager.hpp"
        "${generatedIncludeFiles}"
    sourceFiles
        "${sourcePath}/detail/comparison-operator-utilities.cpp"
        "${sourcePath}/detail/handle-utilities.cpp"
        "${sourcePath}/detail/make-tuple-utilities.cpp"
        "${sourcePath}/detail/reference.cpp"
        "${sourcePath}/detail/structure-copy-utilities.cpp"
        "${sourcePath}/detail/to-string-utilities.cpp"
        "${sourcePath}/context.cpp"
        "${sourcePath}/defaults.cpp"
        "${sourcePath}/format.cpp"
        "${sourcePath}/mesh.cpp"
        "${sourcePath}/render-target.cpp"
        "${sourcePath}/spir-v.cpp"
        "${sourcePath}/wsi-manager.cpp"
        "${generatedSourceFiles}"
)

################################################################################
# gvk.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    target
        gvk
    sourceFiles
        "${testsPath}/comparison-operator.tests.cpp"
        "${testsPath}/reference.tests.cpp"
        "${testsPath}/render-target.tests.cpp"
        "${testsPath}/structure-serialization.tests.cpp"
        "${testsPath}/spir-v.tests.cpp"
        "${testsPath}/structure-copy.tests.cpp"
        "${testsPath}/to-string.tests.cpp"
)
