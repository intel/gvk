
include_guard(GLOBAL)

set(metrics-discovery_VERSION f3b8faa6337ffd52e8e53a3f878543f2e35b7fdc) # metrics-discovery-1.14.181
FetchContent_Declare(
    metrics-discovery
    GIT_REPOSITORY "https://github.com/intel/metrics-discovery.git"
    GIT_TAG ${metrics-discovery_VERSION}
    GIT_PROGRESS TRUE
)

FetchContent_Populate(metrics-discovery)
FetchContent_GetProperties(metrics-discovery SOURCE_DIR metrics-discovery_SOURCE_DIR)
add_library(metrics-discovery INTERFACE)
target_include_directories(metrics-discovery INTERFACE "$<BUILD_INTERFACE:${metrics-discovery_SOURCE_DIR}/inc/>" $<INSTALL_INTERFACE:include>)

if(metrics-discovery_INSTALL_ARTIFACTS)
    gvk_install_artifacts(TARGET metrics-discovery VERSION ${metrics-discovery_VERSION})
endif()
if(metrics-discovery_INSTALL_HEADERS)
    gvk_install_headers(TARGET metrics-discovery)
endif()
