
include_guard(GLOBAL)

include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(CMakeParseArguments)
include(CTest)
include(FetchContent)

find_package(Git REQUIRED)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(gvkBuildModuleDirectory "${CMAKE_CURRENT_LIST_DIR}")

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

function(_gvk_get_directory_targets_impl directory)
    get_property(subdirectories DIRECTORY ${directory} PROPERTY SUBDIRECTORIES)
    foreach(subdirectory IN LISTS subdirectories)
        _gvk_get_directory_targets_impl(${subdirectory})
    endforeach()
    get_property(targets DIRECTORY ${directory} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS targets)
        set(_gvkDirectoryTargets "${_gvkDirectoryTargets}" "${directory}::${target}" CACHE INTERNAL "")
    endforeach()
endfunction()

function(gvk_get_directory_targets directory outTargets)
    set(_gvkDirectoryTargets "" CACHE INTERNAL "")
    _gvk_get_directory_targets_impl(${directory})
    set(${outTargets} "${_gvkDirectoryTargets}" PARENT_SCOPE)
    set(_gvkDirectoryTargets "" CACHE INTERNAL "")
endfunction()

function(gvk_setup_target)
    cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;COMPILE_DEFINITIONS" ${ARGN})
    string(FIND ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/internal/" gvkInternal)
    if(gvkInternal GREATER_EQUAL 0)
        target_include_directories(${ARGS_TARGET} PUBLIC "$<BUILD_INTERFACE:${ARGS_INCLUDE_DIRECTORIES}>" INTERFACE "$<INSTALL_INTERFACE:internal/include>")
    else()
        target_include_directories(${ARGS_TARGET} PUBLIC "$<BUILD_INTERFACE:${ARGS_INCLUDE_DIRECTORIES}>" INTERFACE "$<INSTALL_INTERFACE:include>")
    endif()
    target_compile_definitions(${ARGS_TARGET} PUBLIC "${ARGS_COMPILE_DEFINITIONS}")
    target_link_libraries(${ARGS_TARGET} PUBLIC ${ARGS_LINK_LIBRARIES})
    set_target_properties(${ARGS_TARGET} PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(${ARGS_TARGET} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror -fPIC>)
    gvk_create_file_group("${ARGS_INCLUDE_FILES}")
    gvk_create_file_group("${ARGS_SOURCE_FILES}")
    set_target_properties(${ARGS_TARGET} PROPERTIES FOLDER "${GVK_IDE_FOLDER}/${ARGS_FOLDER}")
    if(gvkInternal GREATER_EQUAL 0)
        set_target_properties(${ARGS_TARGET} PROPERTIES GVK_INTERNAL TRUE)
    endif()

    # NOTE : Disabling constexpr std::mutex ctor to account for mismatch between
    #   the MSVC standard library headers and runtime.  It's not exactly ideal, but
    #   it is the most practical solution for the time being since this project is
    #   distributed in source code form.
    # FROM : https://github.com/microsoft/STL/releases/tag/vs-2022-17.10
    #   Fixed mutex's constructor to be constexpr.
    #   Note: Programs that aren't following the documented restrictions on binary compatibility may encounter null dereferences in mutex machinery. You must follow this rule:
    #   When you mix binaries built by different supported versions of the toolset, the Redistributable version must be at least as new as the latest toolset used by any app component.
    #   You can define _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR as an escape hatch.
    # TODO : As MSVC/Visual Studio receive updates, pay attention to when would be
    #   a good time to remove this...presumably it will be deprecated at some point
    #   anyway.
    if (MSVC_VERSION GREATER_EQUAL 1930)
        target_compile_options(${ARGS_TARGET} PRIVATE -D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
    endif()
endfunction()

function(gvk_add_static_library)
    cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;COMPILE_DEFINITIONS" ${ARGN})
    add_library(${ARGS_TARGET} STATIC "${ARGS_INCLUDE_FILES}" "${ARGS_SOURCE_FILES}")
    gvk_setup_target(
        TARGET               ${ARGS_TARGET}
        FOLDER              "${ARGS_FOLDER}"
        LINK_LIBRARIES       ${ARGS_LINK_LIBRARIES}
        INCLUDE_DIRECTORIES  ${ARGS_INCLUDE_DIRECTORIES}
        INCLUDE_FILES       "${ARGS_INCLUDE_FILES}"
        SOURCE_FILES        "${ARGS_SOURCE_FILES}"
        COMPILE_DEFINITIONS "${ARGS_COMPILE_DEFINITIONS}"
    )
endfunction()

function(gvk_add_executable)
    cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;COMPILE_DEFINITIONS" ${ARGN})
    add_executable(${ARGS_TARGET} "${ARGS_INCLUDE_FILES}" "${ARGS_SOURCE_FILES}")
    gvk_setup_target(
        TARGET               ${ARGS_TARGET}
        FOLDER              "${ARGS_FOLDER}"
        LINK_LIBRARIES       ${ARGS_LINK_LIBRARIES}
        INCLUDE_DIRECTORIES  ${ARGS_INCLUDE_DIRECTORIES}
        INCLUDE_FILES       "${ARGS_INCLUDE_FILES}"
        SOURCE_FILES        "${ARGS_SOURCE_FILES}"
        COMPILE_DEFINITIONS "${ARGS_COMPILE_DEFINITIONS}"
    )
    set_target_properties(${ARGS_TARGET} PROPERTIES GVK_EXECUTABLE TRUE)
endfunction()

function(gvk_add_code_generator)
    cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;INPUT_FILES;OUTPUT_FILES;COMPILE_DEFINITIONS" ${ARGN})
    gvk_add_executable(
        TARGET               ${ARGS_TARGET}
        FOLDER              "${ARGS_FOLDER}"
        LINK_LIBRARIES       ${ARGS_LINK_LIBRARIES}
        INCLUDE_DIRECTORIES  ${ARGS_INCLUDE_DIRECTORIES}
        INCLUDE_FILES       "${ARGS_INCLUDE_FILES}"
        SOURCE_FILES        "${ARGS_SOURCE_FILES}"
        COMPILE_DEFINITIONS "${ARGS_COMPILE_DEFINITIONS}"
    )
    add_custom_command(
        OUTPUT ${ARGS_OUTPUT_FILES}
        COMMAND "${ARGS_TARGET}" "${ARGS_INPUT_FILES}"
        DEPENDS ${ARGS_TARGET} ${ARGS_INPUT_FILES}
    )
endfunction()

function(gvk_add_layer)
    cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INTERFACE_FILES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;COMPILE_DEFINITIONS;DESCRIPTION;VERSION;COMPANY;COPYRIGHT;ENTRY_POINTS" ${ARGN})
    if(NOT ARGS_VERSION)
        set(ARGS_VERSION 1)
    endif()
    if(NOT ARGS_COMPANY)
        set(ARGS_COMPANY "Intel Corporation")
    endif()
    if(NOT ARGS_COPYRIGHT)
        set(ARGS_COPYRIGHT "Copyright Intel Corporation")
    endif()
    if(MSVC)
        string(REPLACE ";" "\n" ARGS_ENTRY_POINTS "${ARGS_ENTRY_POINTS}")
        configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gvk-layer.def.in" "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.def")
        configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gvk-layer.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.rc")
        list(APPEND ARGS_SOURCE_FILES
            "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.def"
            "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.rc"
        )
    endif()
    if(ARGS_INTERFACE_FILES)
        add_library(${ARGS_TARGET}-interface INTERFACE "${ARGS_INTERFACE_FILES}")
        target_link_libraries(${ARGS_TARGET}-interface INTERFACE Vulkan::Vulkan)
        target_include_directories(${ARGS_TARGET}-interface INTERFACE "$<BUILD_INTERFACE:${ARGS_INCLUDE_DIRECTORIES}>" INTERFACE $<INSTALL_INTERFACE:include>)
        gvk_create_file_group("${ARGS_INTERFACE_FILES}")
        set_target_properties(${ARGS_TARGET}-interface PROPERTIES FOLDER "${GVK_IDE_FOLDER}/${ARGS_FOLDER}")
        list(APPEND ARGS_LINK_LIBRARIES ${ARGS_TARGET}-interface)
    endif()
    add_library(${ARGS_TARGET} SHARED "${ARGS_INCLUDE_FILES}" "${ARGS_SOURCE_FILES}")
    list(APPEND ARGS_LINK_LIBRARIES gvk-layer)
    gvk_setup_target(
        TARGET               ${ARGS_TARGET}
        FOLDER              "${ARGS_FOLDER}"
        LINK_LIBRARIES       ${ARGS_LINK_LIBRARIES}
        INCLUDE_DIRECTORIES "${ARGS_INCLUDE_DIRECTORIES}"
        INCLUDE_FILES       "${ARGS_INCLUDE_FILES}"
        SOURCE_FILES        "${ARGS_SOURCE_FILES}"
        COMPILE_DEFINITIONS  ${ARGS_COMPILE_DEFINITIONS}
    )
    set_target_properties(${ARGS_TARGET} PROPERTIES GVK_LAYER TRUE)
    if(MSVC)
        target_compile_options(${ARGS_TARGET} PRIVATE /guard:cf)
        target_link_options(${ARGS_TARGET} PRIVATE /guard:cf /DYNAMICBASE)
        set(libraryPath ".\\\\${ARGS_TARGET}.dll")
    else()
        set(libraryPath "./lib${ARGS_TARGET}.so")
    endif()
    # HUH : Why doesn't CMAKE_CURRENT_FUNCTION_LIST_DIR work on Linux?
    configure_file("${gvkBuildModuleDirectory}/gvk-layer.json.in" "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.json")
    add_custom_command(
        TARGET ${ARGS_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.json" "$<TARGET_FILE_DIR:${ARGS_TARGET}>"
    )
endfunction()

function(gvk_add_pipeline_explorer_plugin)
    cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;COMPILE_DEFINITIONS;DESCRIPTION;VERSION;COMPANY;COPYRIGHT;ENTRY_POINTS" ${ARGN})
    add_library(${ARGS_TARGET} SHARED "${ARGS_INCLUDE_FILES}" "${ARGS_SOURCE_FILES}")
    list(APPEND ARGS_LINK_LIBRARIES gvk-pipeline-explorer-plugin-factory)
    gvk_setup_target(
        TARGET               ${ARGS_TARGET}
        FOLDER              "${ARGS_FOLDER}"
        LINK_LIBRARIES       ${ARGS_LINK_LIBRARIES}
        INCLUDE_DIRECTORIES "${ARGS_INCLUDE_DIRECTORIES}"
        INCLUDE_FILES       "${ARGS_INCLUDE_FILES}"
        SOURCE_FILES        "${ARGS_SOURCE_FILES}"
        COMPILE_DEFINITIONS  ${ARGS_COMPILE_DEFINITIONS}
    )
    set_target_properties(${ARGS_TARGET} PROPERTIES GVK_METRICS_PROVIDER TRUE)
    if(MSVC)
        target_compile_options(${ARGS_TARGET} PRIVATE /guard:cf)
        target_link_options(${ARGS_TARGET} PRIVATE /guard:cf /DYNAMICBASE)
        set(libraryPath ".\\\\${ARGS_TARGET}.dll")
    else()
        set(libraryPath "./lib${ARGS_TARGET}.so")
    endif()
    # HUH : Why doesn't CMAKE_CURRENT_FUNCTION_LIST_DIR work on Linux?
    configure_file("${gvkBuildModuleDirectory}/gvk-pipeline-explorer-plugin.json.in" "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.json")
    add_custom_command(
        TARGET ${ARGS_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.json" "$<TARGET_FILE_DIR:${ARGS_TARGET}>"
    )
endfunction()

macro(gvk_add_target_test)
    if(gvk-build-tests)
        cmake_parse_arguments(ARGS "" "TARGET;FOLDER" "LINK_LIBRARIES;INCLUDE_DIRECTORIES;INCLUDE_FILES;SOURCE_FILES;COMPILE_DEFINITIONS" ${ARGN})
        list(APPEND ARGS_LINK_LIBRARIES gtest gtest_main)
        get_target_property(type ${ARGS_TARGET} TYPE)
        if(type STREQUAL STATIC_LIBRARY)
            list(APPEND ARGS_LINK_LIBRARIES ${ARGS_TARGET})
        elseif(EXISTS ${ARGS_TARGET}-interface)
            list(APPEND ARGS_LINK_LIBRARIES ${ARGS_TARGET}-interface)
        endif()
        gvk_add_executable(
            TARGET               ${ARGS_TARGET}.tests
            FOLDER              "${ARGS_FOLDER}"
            LINK_LIBRARIES       ${ARGS_LINK_LIBRARIES}
            INCLUDE_DIRECTORIES "${ARGS_INCLUDE_DIRECTORIES}"
            INCLUDE_FILES       "${ARGS_INCLUDE_FILES}"
            SOURCE_FILES        "${ARGS_SOURCE_FILES}"
            COMPILE_DEFINITIONS  ${ARGS_COMPILE_DEFINITIONS}
        )

        # NOTE : See the note in gvk_setup_target() for more info
        if (MSVC_VERSION GREATER_EQUAL 1930)
            target_compile_options(${ARGS_TARGET}.tests PRIVATE -D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
        endif()

        if(type STREQUAL SHARED_LIBRARY)
            add_dependencies(${ARGS_TARGET}.tests ${ARGS_TARGET})
        endif()
        if(gvk-run-tests)
            add_test(NAME ${ARGS_TARGET}.tests COMMAND ${ARGS_TARGET}.tests)
            add_custom_command(
                TARGET ${ARGS_TARGET}.tests POST_BUILD
                COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --verbose --output-on-failure
            )
        endif()

        # Add to gvk-test-package
        set(gvk-test-package "${CMAKE_BINARY_DIR}/gvk-test-package/")
        if(NOT EXISTS "${gvk-test-package}")
            file(MAKE_DIRECTORY "${gvk-test-package}")
        endif()
        add_custom_command(
            TARGET ${ARGS_TARGET}.tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${ARGS_TARGET}.tests> "${gvk-test-package}/"
        )
        if(type STREQUAL SHARED_LIBRARY)
            add_dependencies(${ARGS_TARGET}.tests ${ARGS_TARGET})
            add_custom_command(
                TARGET ${ARGS_TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${ARGS_TARGET}> "${gvk-test-package}/"
            )
            if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.json")
                add_custom_command(
                    TARGET ${ARGS_TARGET} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/${ARGS_TARGET}.json" "${gvk-test-package}/"
                )
            endif()
        endif()

        # Create custom target to create gvk-test-package archive
        if(NOT TARGET gvk-test-package-archive)
            add_custom_target(
                gvk-test-package-archive
                COMMAND ${CMAKE_COMMAND} -E tar cvf "${CMAKE_BINARY_DIR}/gvk-test-package.zip" --format=zip "${gvk-test-package}/"
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                COMMENT "TODO : Documentation"
                VERBATIM
            )
        endif()
    endif()
endmacro()

macro(gvk_get_commit_hash commitHash)
    execute_process(
        WORKING_DIRECTORY "${gvkBuildModuleDirectory}/../"
        COMMAND ${GIT_EXECUTABLE} rev-parse --verify HEAD
        OUTPUT_VARIABLE ${commitHash}
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endmacro()

function(gvk_install_target)
    cmake_parse_arguments(ARGS "" "TARGET" "" ${ARGN})
    get_target_property(gvkInternal ${ARGS_TARGET} GVK_INTERNAL)
    if(gvkInternal)
        install(
            TARGETS ${ARGS_TARGET}
            EXPORT ${ARGS_TARGET}Targets
            LIBRARY DESTINATION internal/lib/$<CONFIG>/
            ARCHIVE DESTINATION internal/lib/$<CONFIG>/
            RUNTIME DESTINATION internal/bin/$<CONFIG>/
        )
    else()
        install(
            TARGETS ${ARGS_TARGET}
            EXPORT ${ARGS_TARGET}Targets
            LIBRARY DESTINATION lib/$<CONFIG>/
            ARCHIVE DESTINATION lib/$<CONFIG>/
            RUNTIME DESTINATION bin/$<CONFIG>/
        )
    endif()
endfunction()

function(gvk_install_artifacts)
    cmake_parse_arguments(ARGS "" "TARGET;VERSION" "" ${ARGN})

    # TODO : Documentation
    if(NOT ARGS_VERSION)
        get_target_property(ARGS_VERSION ${ARGS_TARGET} VERSION)
        if(NOT ARGS_VERSION)
            gvk_get_commit_hash(ARGS_VERSION)
        endif()
    endif()

    # TODO : Documentation
    get_target_property(gvkInternal ${ARGS_TARGET} GVK_INTERNAL)

    # TODO : Documentation
    gvk_install_target(TARGET ${ARGS_TARGET})
    export(EXPORT ${ARGS_TARGET}Targets FILE "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}Targets.cmake")

    # TODO : Documentation
    if(gvkInternal)
        install(EXPORT ${ARGS_TARGET}Targets DESTINATION internal/cmake/${ARGS_TARGET}/)
    else()
        install(EXPORT ${ARGS_TARGET}Targets DESTINATION cmake/${ARGS_TARGET}/)
    endif()

    # TODO : Documentation
    set(configVersion "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}ConfigVersion.cmake")
    write_basic_package_version_file("${configVersion}" VERSION ${ARGS_VERSION} COMPATIBILITY ExactVersion)
    set(configTemplate "${gvkBuildModuleDirectory}/gvk-target.config.cmake.in")
    set(config "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}Config.cmake")

    # TODO : Documentation
    if(NOT gvkInternal)
        # NOTE : Get INTERFACE_LINK_LIBRARIES and use it to create a list of targets
        #   the current target depends on.  Remove CMake syntax, system libraries, and
        #   Vulkan.  The resulting list is used on import to ensure all dependencies of
        #   the current target that are defined by the GVK build are imported.
        # NOTE : Kinda kludgy, but INTERFACE_LINK_LIBRARIES seems to be the best option
        get_target_property(interfaceLinkLibraries ${ARGS_TARGET} INTERFACE_LINK_LIBRARIES)
        string(REPLACE "$" "" interfaceLinkLibraries "${interfaceLinkLibraries}")
        string(REPLACE "<LINK_ONLY:" "" interfaceLinkLibraries "${interfaceLinkLibraries}")
        string(REPLACE ">" "" interfaceLinkLibraries "${interfaceLinkLibraries}")
        list(REMOVE_ITEM interfaceLinkLibraries ${CMAKE_DL_LIBS} m rt Threads::Threads Vulkan::Vulkan)
        foreach(interfaceLinkLibrary IN LISTS interfaceLinkLibraries)
            string(FIND "${interfaceLinkLibrary}" "boost" i0)
            string(FIND "${interfaceLinkLibrary}" "libm.a" i1)
            string(FIND "${interfaceLinkLibrary}" "libm.so" i2)
            string(FIND "${interfaceLinkLibrary}" "librt.a" i3)
            string(FIND "${interfaceLinkLibrary}" "librt.so" i4)
            if(i0 GREATER_EQUAL 0 OR i1 GREATER_EQUAL 0 OR i2 GREATER_EQUAL 0 OR i3 GREATER_EQUAL 0 OR i4 GREATER_EQUAL 0)
                list(REMOVE_ITEM interfaceLinkLibraries "${interfaceLinkLibrary}")
            endif()
        endforeach()
    endif()

    # TODO : Documentation
    configure_package_config_file("${configTemplate}" "${config}" INSTALL_DESTINATION {CMAKE_BINARY_DIR}/cmake/)
    if(gvkInternal)
        install(FILES "${config}" "${configVersion}" DESTINATION internal/cmake/${ARGS_TARGET}/)
    else()
        install(FILES "${config}" "${configVersion}" DESTINATION cmake/${ARGS_TARGET}/)
    endif()
endfunction()

function(gvk_install_headers)
    cmake_parse_arguments(ARGS "" "TARGET" "EXCLUDE" ${ARGN})
    get_target_property(interfaceIncludeDirectories ${ARGS_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
    if(interfaceIncludeDirectories)
        string(REPLACE "$" "" interfaceIncludeDirectories "${interfaceIncludeDirectories}")
        string(REPLACE "<BUILD_INTERFACE:" "" interfaceIncludeDirectories "${interfaceIncludeDirectories}")
        string(REPLACE "<INSTALL_INTERFACE:include" "" interfaceIncludeDirectories "${interfaceIncludeDirectories}")
        string(REPLACE "<INSTALL_INTERFACE:internal/include" "" interfaceIncludeDirectories "${interfaceIncludeDirectories}")
        string(REPLACE ">" "" interfaceIncludeDirectories "${interfaceIncludeDirectories}")
        foreach(exclude ${ARGS_EXCLUDE})
            list(APPEND excludeExpression PATTERN "${exclude}" EXCLUDE)
        endforeach()
        get_target_property(gvkInternal ${ARGS_TARGET} GVK_INTERNAL)
        foreach(interfaceIncludeDirectory ${interfaceIncludeDirectories})
            if(gvkInternal)
                install(DIRECTORY "${interfaceIncludeDirectory}" DESTINATION internal/include/ ${excludeExpression})
            else()
                install(DIRECTORY "${interfaceIncludeDirectory}" DESTINATION include/ ${excludeExpression})
            endif()
        endforeach()
    endif()
endfunction()

function(gvk_install_layer)
    cmake_parse_arguments(ARGS "" "TARGET;VERSION" "" ${ARGN})
    if(NOT ARGS_VERSION)
        get_target_property(ARGS_VERSION ${ARGS_TARGET} VERSION)
        if(NOT ARGS_VERSION)
            gvk_get_commit_hash(ARGS_VERSION)
        endif()
    endif()

    # TODO : Documentation
    get_target_property(gvkInternal ${ARGS_TARGET} GVK_INTERNAL)

    # TODO : Documentation
    gvk_install_target(TARGET ${ARGS_TARGET})
    set(configVersion "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}ConfigVersion.cmake")

    # TODO : Documentation
    write_basic_package_version_file("${configVersion}" VERSION ${ARGS_VERSION} COMPATIBILITY ExactVersion)
    set(configTemplate "${gvkBuildModuleDirectory}/gvk-target.config.cmake.in")
    set(config "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}Config.cmake")
    configure_package_config_file("${configTemplate}" "${config}" INSTALL_DESTINATION {CMAKE_BINARY_DIR}/cmake/)

    # TODO : Documentation
    if(gvkInternal)
        install(FILES "${config}" "${configVersion}" DESTINATION internal/cmake/${ARGS_TARGET}/)
        install(FILES "$<TARGET_FILE_DIR:${ARGS_TARGET}>/${ARGS_TARGET}.json" DESTINATION internal/bin/$<CONFIG>/)
    else()
        install(FILES "${config}" "${configVersion}" DESTINATION cmake/${ARGS_TARGET}/)
        install(FILES "$<TARGET_FILE_DIR:${ARGS_TARGET}>/${ARGS_TARGET}.json" DESTINATION bin/$<CONFIG>/)
    endif()
endfunction()

function(gvk_install_pipeline_explorer_plugin)
    cmake_parse_arguments(ARGS "" "TARGET;VERSION" "" ${ARGN})
    if(NOT ARGS_VERSION)
        get_target_property(ARGS_VERSION ${ARGS_TARGET} VERSION)
        if(NOT ARGS_VERSION)
            gvk_get_commit_hash(ARGS_VERSION)
        endif()
    endif()

    # TODO : Documentation
    get_target_property(gvkInternal ${ARGS_TARGET} GVK_INTERNAL)

    # TODO : Documentation
    gvk_install_target(TARGET ${ARGS_TARGET})
    set(configVersion "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}ConfigVersion.cmake")

    # TODO : Documentation
    write_basic_package_version_file("${configVersion}" VERSION ${ARGS_VERSION} COMPATIBILITY ExactVersion)
    set(configTemplate "${gvkBuildModuleDirectory}/gvk-target.config.cmake.in")
    set(config "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}Config.cmake")
    configure_package_config_file("${configTemplate}" "${config}" INSTALL_DESTINATION {CMAKE_BINARY_DIR}/cmake/)

    # TODO : Documentation
    if(gvkInternal)
        install(FILES "${config}" "${configVersion}" DESTINATION internal/cmake/${ARGS_TARGET}/)
        install(FILES "$<TARGET_FILE_DIR:${ARGS_TARGET}>/${ARGS_TARGET}.json" DESTINATION internal/bin/$<CONFIG>/)
    else()
        install(FILES "${config}" "${configVersion}" DESTINATION cmake/${ARGS_TARGET}/)
        install(FILES "$<TARGET_FILE_DIR:${ARGS_TARGET}>/${ARGS_TARGET}.json" DESTINATION bin/$<CONFIG>/)
    endif()
endfunction()

function(gvk_install_metrics_provider)
    cmake_parse_arguments(ARGS "" "TARGET;VERSION" "" ${ARGN})
    if(NOT ARGS_VERSION)
        get_target_property(ARGS_VERSION ${ARGS_TARGET} VERSION)
        if(NOT ARGS_VERSION)
            gvk_get_commit_hash(ARGS_VERSION)
        endif()
    endif()

    # TODO : Documentation
    get_target_property(gvkInternal ${ARGS_TARGET} GVK_INTERNAL)

    # TODO : Documentation
    gvk_install_target(TARGET ${ARGS_TARGET})
    set(configVersion "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}ConfigVersion.cmake")

    # TODO : Documentation
    write_basic_package_version_file("${configVersion}" VERSION ${ARGS_VERSION} COMPATIBILITY ExactVersion)
    set(configTemplate "${gvkBuildModuleDirectory}/gvk-target.config.cmake.in")
    set(config "${CMAKE_BINARY_DIR}/cmake/${ARGS_TARGET}Config.cmake")
    configure_package_config_file("${configTemplate}" "${config}" INSTALL_DESTINATION {CMAKE_BINARY_DIR}/cmake/)

    # TODO : Documentation
    if(gvkInternal)
        install(FILES "${config}" "${configVersion}" DESTINATION internal/cmake/${ARGS_TARGET}/)
        install(FILES "$<TARGET_FILE_DIR:${ARGS_TARGET}>/${ARGS_TARGET}.json" DESTINATION internal/bin/$<CONFIG>/)
    else()
        install(FILES "${config}" "${configVersion}" DESTINATION cmake/${ARGS_TARGET}/)
        install(FILES "$<TARGET_FILE_DIR:${ARGS_TARGET}>/${ARGS_TARGET}.json" DESTINATION bin/$<CONFIG>/)
    endif()
endfunction()

function(gvk_install_executable)
    cmake_parse_arguments(ARGS "" "TARGET;VERSION" "" ${ARGN})
    if(NOT ARGS_VERSION)
        get_target_property(ARGS_VERSION ${ARGS_TARGET} VERSION)
        if(NOT ARGS_VERSION)
            gvk_get_commit_hash(ARGS_VERSION)
        endif()
    endif()
    gvk_install_target(TARGET ${ARGS_TARGET})
endfunction()

function(gvk_create_package_config)
    cmake_parse_arguments(ARGS "" "NAME;DESTINATION" "TARGETS" ${ARGN})
    if(ARGS_TARGETS)
        gvk_get_commit_hash(version)
        set(configVersion "${CMAKE_BINARY_DIR}/cmake/${ARGS_NAME}ConfigVersion.cmake")
        write_basic_package_version_file("${configVersion}" VERSION ${version} COMPATIBILITY ExactVersion)
        set(configTemplate "${gvkBuildModuleDirectory}/gvk.config.cmake.in")
        set(config "${CMAKE_BINARY_DIR}/cmake/${ARGS_NAME}Config.cmake")
        configure_package_config_file("${configTemplate}" "${config}" INSTALL_DESTINATION "${CMAKE_BINARY_DIR}/${ARGS_DESTINATION}")
        set(gvkVersion "${CMAKE_BINARY_DIR}/cmake/${ARGS_NAME}Version.cmake")
        file(WRITE "${gvkVersion}" "${version}")
        install(FILES "${config}" "${configVersion}" "${gvkVersion}" DESTINATION "${ARGS_DESTINATION}")
    endif()
endfunction()

function(gvk_install_package)
    file(GLOB exportedConfigFiles "${CMAKE_BINARY_DIR}/cmake/*Config.cmake")
    foreach(exportedConfigFile ${exportedConfigFiles})
        get_filename_component(exportedConfigFileName "${exportedConfigFile}" NAME)
        string(REPLACE "Config.cmake" "" exportedTargetName "${exportedConfigFileName}")
        if(TARGET ${exportedTargetName})
            get_target_property(gvkLayer ${exportedTargetName} GVK_LAYER)
            get_target_property(gvkExecutable ${exportedTargetName} GVK_EXECUTABLE)
            get_target_property(gvkInternal ${exportedTargetName} GVK_INTERNAL)
            if(NOT gvkLayer AND NOT gvkExecutable)
                if(gvkInternal)
                    list(APPEND internalExportedTargets ${exportedTargetName})
                else()
                    list(APPEND exportedTargets ${exportedTargetName})
                endif()
            endif()
        endif()
    endforeach()
    gvk_create_package_config(NAME gvk-internal TARGETS ${internalExportedTargets} DESTINATION internal/cmake)
    gvk_create_package_config(NAME gvk          TARGETS ${exportedTargets}         DESTINATION cmake)
endfunction()
