
################################################################################
# gvk-handles.generator
set(generatedIncludeDirectory "${CMAKE_CURRENT_BINARY_DIR}/include/")
set(generatedIncludePath "${generatedIncludeDirectory}/gvk-handles/generated/")
set(generatedSourcePath "${CMAKE_CURRENT_BINARY_DIR}/source/gvk-handles/generated/")
set(generatedIncludeFiles
    "${generatedIncludePath}/forward-declarations.inl"
    "${generatedIncludePath}/handles.hpp"
)
set(generatedSourceFiles
    "${generatedSourcePath}/handles.cpp"
)
set(generatorSourcePath "${CMAKE_CURRENT_LIST_DIR}/generator/")
gvk_add_code_generator(
    target
        gvk-handles.generator
    folder
        "gvk-handles/"
    linkLibraries
        gvk-cppgen
    includeDirectories
        "${generatorSourcePath}/"
    includeFiles
        "${generatorSourcePath}/handles.generator.hpp"
    sourceFiles
        "${generatorSourcePath}/main.cpp"
    outputFiles
        "${generatedIncludeFiles}"
        "${generatedSourceFiles}"
    compileDefinitions
        GVK_HANDLES_GENERATED_INCLUDE_PATH="${generatedIncludePath}"
        GVK_HANDLES_GENERATED_INCLUDE_PREFIX="gvk-handles/generated/"
        GVK_HANDLES_GENERATED_SOURCE_PATH="${generatedSourcePath}"
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)
if(MSVC)
    set_source_files_properties("${generatedSourcePath}/comparison-operators.cpp" PROPERTIES COMPILE_FLAGS "/bigobj")
endif()

################################################################################
# gvk-handles
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-handles/")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-handles/")
gvk_add_static_library(
    target
        gvk-handles
    folder
        "gvk-handles/"
    linkLibraries
        gvk-format-info
        gvk-reference
        gvk-runtime
        gvk-string
        gvk-structures
        VulkanMemoryAllocator
    includeDirectories
        "${generatedIncludeDirectory}"
        "${includeDirectory}"
    includeFiles
        "${generatedIncludeFiles}"
        "${includePath}/detail/handle-utilities.hpp"
        "${includePath}/context.hpp"
        "${includePath}/defines.hpp"
        "${includePath}/handles.hpp"
        "${includePath}/mesh.hpp"
        "${includePath}/render-target.hpp"
        "${includePath}/utilities.hpp"
        "${includePath}/wsi-manager.hpp"
        "${includeDirectory}/gvk-handles.hpp"
    sourceFiles
        "${generatedSourceFiles}"
        "${sourcePath}/detail/handle-utilities.cpp"
        "${sourcePath}/context.cpp"
        "${sourcePath}/mesh.cpp"
        "${sourcePath}/render-target.cpp"
        "${sourcePath}/utilities.cpp"
        "${sourcePath}/wsi-manager.cpp"
)

################################################################################
# gvk-handles.test
set(testsPath "${CMAKE_CURRENT_LIST_DIR}/tests/")
gvk_add_target_test(
    target
        gvk-handles
    folder
        "gvk-handles/"
    sourceFiles
        "${testsPath}/render-target.tests.cpp"
)
