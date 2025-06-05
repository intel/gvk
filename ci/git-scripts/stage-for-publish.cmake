
include(git-utilities.cmake)

if(CMAKE_ARGC LESS_EQUAL 5)
    message("Usage : cmake -P stage-for-publish.cmake <publish directory>")
else()
    set(publishDirectory "${CMAKE_ARGV3}")
    set(internalRepoBase "${CMAKE_ARGV4}")
    set(internalRepoName "${CMAKE_ARGV5}")

    # Clean the publish directory
    file(REMOVE_RECURSE "${publishDirectory}")
    file(MAKE_DIRECTORY "${publishDirectory}")

    # Clone the internal repo, get the version, and prepare for read
    git_clone_repo("${publishDirectory}" "${internalRepoBase}" "${internalRepoName}")
    git_get_version("${publishDirectory}" "${internalRepoBase}" "${internalRepoName}" version)
    git_prepare_repo_for_read("${publishDirectory}" "${internalRepoBase}" "${internalRepoName}")

    # Clone the public repo, update the version, and prepare for write
    git_clone_repo("${publishDirectory}" "intel" "gvk")
    file(WRITE "${publishDirectory}/intel/gvk/INTERNAL-VERSION.txt" ${version})
    git_prepare_repo_for_write("${publishDirectory}" "intel" "gvk")

    # Copy the contents of the internal repo into the public repo
    file(GLOB directoryEntries "${publishDirectory}/${internalRepoBase}/${internalRepoName}/*")
    file(COPY ${directoryEntries} DESTINATION "${publishDirectory}/intel/gvk/")
endif()
