
# NOTE : Git, Python, and git-filter-repo must be installed and added to PATH
#   https://github.com/newren/git-filter-repo
#   https://www.git-tower.com/learn/git/faq/git-filter-repo
#   python -m pip install git-filter-repo

find_package(Git REQUIRED)

if(CMAKE_ARGC LESS_EQUAL 3)
    message("Usage : cmake -P cherry-pick-to-public.cmake <publish directory>")
else()
    set(publishDirectory "${CMAKE_ARGV3}")
    set(internalRepoBase "${CMAKE_ARGV4}")
    set(internalRepoName "${CMAKE_ARGV5}")

    # Clean the publish directory
    file(REMOVE_RECURSE "${publishDirectory}")
    file(MAKE_DIRECTORY "${publishDirectory}")

    # Clone public repo
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/"
        COMMAND ${GIT_EXECUTABLE} clone "https://github.com/intel/gvk.git" "gvk-public"
    )

    # Clone private repo
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/"
        COMMAND ${GIT_EXECUTABLE} clone "https://github.com/${internalRepoBase}/${internalRepoName}.git" "gvk-private"
    )

    # Get private repo version
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/gvk-private/"
        COMMAND ${GIT_EXECUTABLE} rev-parse --verify HEAD
        OUTPUT_VARIABLE version
    )
    string(SUBSTRING ${version} 0 7 shortVersion)

    # Get private repo version that public repo currently reflects
    file(READ "${publishDirectory}/gvk-public/INTERNAL-VERSION.txt" previousVersion)
    string(STRIP "${previousVersion}" previousVersion)

    # Create publish branch from public trunk
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/gvk-public/"
        COMMAND ${GIT_EXECUTABLE} checkout -b publish-${shortVersion} trunk
    )

    # Get commits from [previousVersion (exclusive) to version (inclusive))
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/gvk-private/"
        COMMAND ${GIT_EXECUTABLE} rev-list --reverse ${previousVersion}..HEAD
        OUTPUT_VARIABLE commitList
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REPLACE "\n" ";" commitList "${commitList}")

    # Filter private repo (removes build-support/, internal/, JenkinsFile)
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/gvk-private/"
        COMMAND ${GIT_EXECUTABLE} filter-repo --path build-support/ --path internal/ --path JenkinsFile --invert-paths
    )

    # Generate commit-map
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/gvk-private/"
        COMMAND ${GIT_EXECUTABLE} filter-repo --analyze
    )

    # Read commit-map
    file(READ "${publishDirectory}/gvk-private/.git/filter-repo/commit-map" commitMapRaw)
    string(REPLACE "\n" ";" commitMapLines "${commitMapRaw}")

    # Map original commit SHAs to filtered commit SHAs
    set(commitMap)
    foreach(line IN LISTS commitMapLines)
        string(REGEX MATCH "([a-f0-9]+) ([a-f0-9]+)" _ "${line}")
        set(originalSha "${CMAKE_MATCH_1}")
        set(filteredSha "${CMAKE_MATCH_2}")
        set(commitMap_${originalSha} "${filteredSha}")
    endforeach()

    # Add filtered private repo as remote to public repo
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/gvk-public/"
        COMMAND ${GIT_EXECUTABLE} remote add filtered-${shortVersion} "${publishDirectory}/gvk-private"
    )

    # Fetch trunk from filtered private repo
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/gvk-public/"
        COMMAND ${GIT_EXECUTABLE} fetch filtered-${shortVersion} trunk:filtered-${shortVersion}/trunk
    )

    # Set GIT_EDITOR to suppress editor prompts
    if(CMAKE_HOST_WIN32)
        set(ENV{GIT_EDITOR} echo)
    elseif(CMAKE_HOST_UNIX)
        set(ENV{GIT_EDITOR} true)
    endif()

    # Process commits
    foreach(commit IN LISTS commitList)
        message("")
        message("Processing commit ${commit}")
        set(filteredCommit "${commitMap_${commit}}")
        if(NOT ${filteredCommit} MATCHES "^0+$")

            # Cherry pick filtered commit
            message("cherry-picking filtered commit ${filteredCommit}")
            execute_process(
                WORKING_DIRECTORY "${publishDirectory}/gvk-public/"
                COMMAND ${GIT_EXECUTABLE} cherry-pick --keep-redundant-commits --strategy=recursive -X theirs ${filteredCommit}
                RESULT_VARIABLE cherryPickResult
            )

            # If unsuccessful, auto-resolve conflicts taking incoming commit ("theirs")
            if(cherryPickResult)
                execute_process(
                    WORKING_DIRECTORY "${publishDirectory}/gvk-public/"
                    COMMAND ${GIT_EXECUTABLE} checkout --theirs .
                )
                execute_process(
                    WORKING_DIRECTORY "${publishDirectory}/gvk-public/"
                    COMMAND ${GIT_EXECUTABLE} add -A
                )
                execute_process(
                    WORKING_DIRECTORY "${publishDirectory}/gvk-public/"
                    COMMAND ${GIT_EXECUTABLE} cherry-pick --continue
                    RESULT_VARIABLE cherryPickResult
                )
                if(cherryPickResult)
                    message(FATAL_ERROR "git cherry-pick --continue failed with code ${cherryPickResult}")
                endif()
            endif()
        else()
            message("No associated filtered commit to cherry-pick; commit likely contains exclusively filtered content")
        endif()
    endforeach()
    message("")

    # Update, add, and commit INTERNAL-VERSION.txt
    file(WRITE "${publishDirectory}/gvk-public/INTERNAL-VERSION.txt" "${version}")
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/gvk-public/"
        COMMAND ${GIT_EXECUTABLE} add "INTERNAL-VERSION.txt"
    )
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/gvk-public/"
        COMMAND ${GIT_EXECUTABLE} commit -m "${version}"
    )

    # Push publish branch to public repo
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/gvk-public/"
        COMMAND ${GIT_EXECUTABLE} push origin publish-${shortVersion}
    )

    if(0)
    # NOTE : Pushing tags winds up orphaning them since the merge into the public
    #   repo is done via rebase which creates new commit hashes
    # TODO : Figure out how to automate tag creation so that auto-generated release
    #    notes can be used, and to match release/tags between private/public repos
    # Push tags to public repo
    execute_process(
        WORKING_DIRECTORY "${publishDirectory}/gvk-public/"
        COMMAND ${GIT_EXECUTABLE} push --tags origin
    )
    endif()
endif()
