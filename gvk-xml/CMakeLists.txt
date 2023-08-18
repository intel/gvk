
################################################################################
# gvk-xml
set(includeDirectory "${CMAKE_CURRENT_LIST_DIR}/include/")
set(includePath "${includeDirectory}/gvk-xml")
set(sourcePath "${CMAKE_CURRENT_LIST_DIR}/source/gvk-xml/")
gvk_add_static_library(
    TARGET
        gvk-xml
    FOLDER
        "gvk-xml/"
    LINK_LIBRARIES
        gvk-string
        tinyxml2
    INCLUDE_DIRECTORIES
        "${includeDirectory}"
    INCLUDE_FILES
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
        "${includeDirectory}/gvk-xml.hpp"
    SOURCE_FILES
        "${sourcePath}/api-element.cpp"
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
)

################################################################################
# gvk.xml-to-json
gvk_add_executable(
    TARGET
        gvk-xml-to-json
    FOLDER
        "gvk-xml/"
    LINK_LIBRARIES
        gvk-xml
    SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/gvk-xml-to-json.cpp"
    COMPILE_DEFINITIONS
        GVK_XML_FILE_PATH="${Vulkan_XML}"
)
add_custom_target(gvk-xml-to-json.runner ALL COMMAND gvk-xml-to-json > gvk.json)
set_target_properties(gvk-xml-to-json.runner PROPERTIES FOLDER "${GVK_IDE_FOLDER}/gvk-xml/")

################################################################################
# gvk-xml install
gvk_install_library(TARGET gvk-xml)
