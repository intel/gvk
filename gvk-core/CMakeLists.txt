
################################################################################
# gvk-core.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/dispatch-table.hpp"
    "${generatedIncludePath}/format-utilities.hpp"
    "${generatedIncludePath}/handles.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/dispatch-table.cpp"
    "${generatedSourcePath}/format-utilities.cpp"
    "${generatedSourcePath}/handles.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    target
        gvk-core.generator
    folder
        "gvk-core/"
    linkLibraries
        gvk-cppgen
    includeDirectories
        "${generatorSourcePath}/"
    includeFiles
        "${generatorSourcePath}/dispatch-table.generator.hpp"
        "${generatorSourcePath}/format-utilities.generator.hpp"
        "${generatorSourcePath}/handles.generator.hpp"
    sourceFiles
        "${generatorSourcePath}/main.cpp"
    outputFiles
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    compileDefinitions
        GVK_CORE_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_CORE_GENERATED_INCLUDE_PREFIX="gvk/generated/"
        GVK_CORE_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)
if(MSVC)
    set_source_files_properties("${generatedSourcePath}/comparison-operators.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
endif()

################################################################################
# gvk-core
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk/")
gvk_add_static_library(
    target
        gvk-core
    folder
        "gvk-core/"
    linkLibraries
        gvk-reference
        gvk-string
        gvk-structures
        VulkanMemoryAllocator
        Vulkan::Vulkan
        ${CMAKE_DL_LIBS}
    includeDirectories
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    includeFiles
        "${generatedIncludeFiles}"
        "${includePath}/detail/handle-utilities.hpp"
        "${includePath}/context.hpp"
        "${includePath}/defines.hpp"
        "${includePath}/environment.hpp"
        "${includePath}/format.hpp"
        "${includePath}/handles.hpp"
        "${includePath}/mesh.hpp"
        "${includePath}/render-target.hpp"
        "${includePath}/serialization.hpp"
        "${includePath}/structures.hpp"
        "${includePath}/to-string.hpp"
        "${includePath}/utilities.hpp"
        "${includePath}/wsi-manager.hpp"
    sourceFiles
        "${generatedSourceFiles}"
        "${sourcePath}/detail/handle-utilities.cpp"
        "${sourcePath}/context.cpp"
        "${sourcePath}/environment.cpp"
        "${sourcePath}/format.cpp"
        "${sourcePath}/mesh.cpp"
        "${sourcePath}/render-target.cpp"
        "${sourcePath}/utilities.cpp"
        "${sourcePath}/wsi-manager.cpp"
)

################################################################################
# gvk-core.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    target
        gvk-core
    folder
        "gvk-core/"
    sourceFiles
        "${testsPath}/render-target.tests.cpp"
)
