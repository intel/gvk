
include_guard(GLOBAL)

find_package(Git REQUIRED)

set(repoSpecificDirectoryEntries
    ".git"
    "internal/"
    "INTERNAL-VERSION.txt"
)

function(git_clone_repo directory repoType repoName)
    file(REMOVE_RECURSE "${directory}/${repoType}")
    file(MAKE_DIRECTORY "${directory}/${repoType}")
    execute_process(
        WORKING_DIRECTORY "${directory}/${repoType}/"
        COMMAND ${GIT_EXECUTABLE} clone "https://github.com/${repoType}/${repoName}.git"
    )
endfunction()

function(git_checkout directory repoType repoName version)
    execute_process(
        WORKING_DIRECTORY "${directory}/${repoType}/${repoName}/"
        COMMAND ${GIT_EXECUTABLE} checkout ${version}
    )
endfunction()

function(git_get_version directory repoType repoName version)
    message("${directory}/${repoType}/${repoName}/")
    execute_process(
        WORKING_DIRECTORY "${directory}/${repoType}/${repoName}/"
        COMMAND ${GIT_EXECUTABLE} rev-parse --verify HEAD
        OUTPUT_VARIABLE version
    )
    set(version ${version} PARENT_SCOPE)
endfunction()

function(git_prepare_repo_for_read directory repoType repoName)
    foreach(repoSpecificDirectoryEntry ${repoSpecificDirectoryEntries})
        file(REMOVE_RECURSE "${directory}/${repoType}/${repoName}/${repoSpecificDirectoryEntry}")
    endforeach()
endfunction()

function(git_prepare_repo_for_write directory repoType repoName)
    file(GLOB directoryEntries "${directory}/${repoType}/${repoName}/*")
    foreach(repoSpecificDirectoryEntry ${repoSpecificDirectoryEntries})
        list(REMOVE_ITEM directoryEntries "${directory}/${repoType}/${repoName}/${repoSpecificDirectoryEntry}")
    endforeach()
    file(REMOVE_RECURSE ${directoryEntries})
endfunction()
