
include_guard()

include(CMakeParseArguments)
include(CTest)

set(GVK_BUILD_MODULE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

function(gvk_create_file_group files)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    foreach(file ${files})
        get_filename_component(directory "${file}" DIRECTORY)
        string(REPLACE "${PROJECT_SOURCE_DIR}" "" groupName "${directory}")
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" groupName "${groupName}")
        if(MSVC)
            string(REPLACE "/" "\\" groupName "${groupName}")
        endif()
        source_group("${groupName}" FILES "${file}")
    endforeach()
endfunction()

macro(gvk_set_target_option target option)
    if(${option})
        target_compile_definitions(${target} PUBLIC ${option})
    endif()
endmacro()

function(gvk_setup_target)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;compileDefinitions" ${ARGN})
    target_include_directories(${args_target} PUBLIC "${args_includeDirectories}")
    target_compile_definitions(${args_target} PUBLIC "${args_compileDefinitions}")
    target_link_libraries(${args_target} PUBLIC "${args_linkLibraries}")
    set_target_properties(${args_target} PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(${args_target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror -fPIC>)
    gvk_create_file_group("${args_includeFiles}")
    gvk_create_file_group("${args_sourceFiles}")
    if(GVK_NO_PROTOTYPES)
        target_compile_definitions(${args_target} PUBLIC VK_NO_PROTOTYPES)
    endif()
    set_target_properties(${args_target} PROPERTIES FOLDER "${GVK_IDE_FOLDER}/${args_folder}")
endfunction()

function(gvk_add_static_library)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;compileDefinitions" ${ARGN})
    add_library(${args_target} STATIC "${args_includeFiles}" "${args_sourceFiles}")
    gvk_setup_target(
        target              ${args_target}
        folder             "${args_folder}"
        linkLibraries       ${args_linkLibraries}
        includeDirectories "${args_includeDirectories}"
        includeFiles       "${args_includeFiles}"
        sourceFiles        "${args_sourceFiles}"
        compileDefinitions  ${args_compileDefinitions}
    )
endfunction()

function(gvk_add_executable)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;compileDefinitions" ${ARGN})
    add_executable(${args_target} "${args_includeFiles}" "${args_sourceFiles}")
    gvk_setup_target(
        target              ${args_target}
        folder             "${args_folder}"
        linkLibraries       ${args_linkLibraries}
        includeDirectories "${args_includeDirectories}"
        includeFiles       "${args_includeFiles}"
        sourceFiles        "${args_sourceFiles}"
        compileDefinitions  ${args_compileDefinitions}
    )
endfunction()

function(gvk_add_code_generator)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;inputFiles;outputFiles;compileDefinitions" ${ARGN})
    gvk_add_executable(
        target              ${args_target}
        folder             "${args_folder}"
        linkLibraries       ${args_linkLibraries}
        includeDirectories "${args_includeDirectories}"
        includeFiles       "${args_includeFiles}"
        sourceFiles        "${args_sourceFiles}"
        compileDefinitions  ${args_compileDefinitions}
    )
    add_custom_command(
        OUTPUT ${args_outputFiles}
        COMMAND "${args_target}" "${args_inputFiles}"
        DEPENDS ${args_target} ${args_inputFiles}
    )
endfunction()

function(gvk_add_layer)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;compileDefinitions;description;version;company;copyright;entryPoints" ${ARGN})
    if(NOT args_version)
        set(args_version 1)
    endif()
    if(NOT args_company)
        set(args_company "Intel Corporation")
    endif()
    if(NOT args_copyright)
        set(args_copyright "Copyright Intel Corporation")
    endif()
    if(MSVC)
        string(REPLACE ";" "\n" args_entryPoints "${args_entryPoints}")
        configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gvk-layer.def.in" "${CMAKE_CURRENT_BINARY_DIR}/${args_target}.def")
        configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gvk-layer.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/${args_target}.rc")
        list(APPEND args_sourceFiles
            "${CMAKE_CURRENT_BINARY_DIR}/${args_target}.def"
            "${CMAKE_CURRENT_BINARY_DIR}/${args_target}.rc"
        )
    endif()
    add_library(${args_target} SHARED "${args_includeFiles}" "${args_sourceFiles}")
    list(APPEND args_linkLibraries gvk-layer)
    gvk_setup_target(
        target              ${args_target}
        folder             "${args_folder}"
        linkLibraries       ${args_linkLibraries}
        includeDirectories "${args_includeDirectories}"
        includeFiles       "${args_includeFiles}"
        sourceFiles        "${args_sourceFiles}"
        compileDefinitions  ${args_compileDefinitions}
    )
    if(MSVC)
        target_compile_options(${args_target} PRIVATE /guard:cf)
        target_link_options(${args_target} PRIVATE /guard:cf /DYNAMICBASE)
        set(libraryPath ".\\\\${args_target}.dll")
    else()
        set(libraryPath "./lib${args_target}.so")
    endif()
    # HUH : Why doesn't CMAKE_CURRENT_FUNCTION_LIST_DIR work on Linux?
    configure_file("${GVK_BUILD_MODULE_DIRECTORY}/gvk-layer.json.in" "${CMAKE_CURRENT_BINARY_DIR}/${args_target}.json")
    add_custom_command(
        TARGET ${args_target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/${args_target}.json" "$<TARGET_FILE_DIR:${args_target}>"
    )
endfunction()

macro(gvk_add_target_test)
    cmake_parse_arguments(args "" "target;folder" "linkLibraries;includeDirectories;includeFiles;sourceFiles;compileDefinitions" ${ARGN})
    if(GVK_BUILD_TESTS)
        list(APPEND args_linkLibraries gtest_main)
        get_target_property(type ${args_target} TYPE)
        if(type STREQUAL STATIC_LIBRARY)
            list(APPEND args_linkLibraries ${args_target})
        endif()
        gvk_add_executable(
            target ${args_target}.tests
            folder ${args_folder}
            linkLibraries ${args_linkLibraries}
            includeDirectories "${args_includeDirectories}"
            includeFiles "${args_includeFiles}"
            sourceFiles "${args_sourceFiles}"
            compileDefinitions ${args_compileDefinitions}
        )
        if(type STREQUAL SHARED_LIBRARY)
            add_dependencies(${args_target}.tests ${args_target})
        endif()
        if(GVK_RUN_TESTS)
            add_test(NAME ${args_target}.tests COMMAND ${args_target}.tests)
            add_custom_command(
                TARGET ${args_target}.tests POST_BUILD
                COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --verbose --output-on-failures
            )
        endif()
        if(GVK_CREATE_TEST_PACKAGE)
            set(package "${CMAKE_BINARY_DIR}/gvk-test-package/")
            if(NOT EXISTS "${package}")
                file(MAKE_DIRECTORY "${package}")
            endif()
            add_custom_command(
                TARGET ${args_target}.tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${args_target}.tests> "${package}/"
            )
            if(type STREQUAL SHARED_LIBRARY)
                add_custom_command(
                    TARGET ${args_target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${args_target}> "${package}/"
                )
                if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${args_target}.json")
                    add_custom_command(
                        TARGET ${args_target} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/${args_target}.json" "${package}/"
                    )
                endif()
            endif()
        endif()
    endif()
endmacro()
