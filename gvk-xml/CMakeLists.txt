
################################################################################
# gvk-xml
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk/xml")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk/xml/")
gvk_add_static_library(
    target
        gvk-xml
    linkLibraries
        gvk-string
        tinyxml2
    includeDirectories
        "${includeDirectory}"
    includeFiles
        "${includePath}/api-element.hpp"
        "${includePath}/command.hpp"
        "${includePath}/defines.hpp"
        "${includePath}/enumeration.hpp"
        "${includePath}/extension.hpp"
        "${includePath}/feature.hpp"
        "${includePath}/format.hpp"
        "${includePath}/handle.hpp"
        "${includePath}/manifest.hpp"
        "${includePath}/parameter.hpp"
        "${includePath}/platform.hpp"
        "${includePath}/structure.hpp"
    sourceFiles
        "${sourcePath}/command.cpp"
        "${sourcePath}/enumeration.cpp"
        "${sourcePath}/extension.cpp"
        "${sourcePath}/feature.cpp"
        "${sourcePath}/format.cpp"
        "${sourcePath}/handle.cpp"
        "${sourcePath}/manifest.cpp"
        "${sourcePath}/parameter.cpp"
        "${sourcePath}/platform.cpp"
        "${sourcePath}/structure.cpp"
        "${sourcePath}/tinyxml2-utilities.hpp"
    compileDefinitions
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)

################################################################################
# gvk.xml-to-json
gvk_add_executable(
    target
        gvk.xml-to-json
    folder
        "tools/"
    linkLibraries
        gvk-xml
    sourceFiles
        "${CMAKE_CURRENT_LIST_DIR}/gvk.xml-to-json.cpp"
)
add_custom_target(
  gvk.xml-to-json.runner ALL
  COMMAND gvk.xml-to-json > gvk.json
)
set_target_properties(gvk.xml-to-json.runner PROPERTIES FOLDER "gvk/tools/")
