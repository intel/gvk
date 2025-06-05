
include(git-utilities.cmake)

if(CMAKE_ARGC LESS_EQUAL 6)
    message("Usage : cmake -P checkout-public-version.cmake <checkout directory> <public version>")
else()
    set(checkoutDirectory "${CMAKE_ARGV3}")
    set(publicVersion "${CMAKE_ARGV4}")
    set(internalRepoBase "${CMAKE_ARGV5}")
    set(internalRepoName "${CMAKE_ARGV6}")

    # Clean the checkout directory
    file(REMOVE_RECURSE "${checkoutDirectory}")
    file(MAKE_DIRECTORY "${checkoutDirectory}")

    # Clone the public repo, checkout the requested public version, get the internal
    #   version referenced by the public snapshot, and prepare for read
    git_clone_repo("${checkoutDirectory}" "intel" "gvk")
    git_checkout("${checkoutDirectory}" "intel" "gvk" "${publicVersion}")
    file(STRINGS "${checkoutDirectory}/intel/gvk/INTERNAL-VERSION.txt" internalVersion)
    git_prepare_repo_for_read("${checkoutDirectory}" "intel" "gvk")

    # Clone the internal repo, checkout the internal version referenced by the
    #   public snapshot, and prepare for write
    git_clone_repo("${checkoutDirectory}" "${internalRepoBase}" "${internalRepoName}")
    git_checkout("${checkoutDirectory}" "${internalRepoBase}" "${internalRepoName}" "${internalVersion}")
    git_prepare_repo_for_write("${checkoutDirectory}" "${internalRepoBase}" "${internalRepoName}")

    # Checkout internal/
    execute_process(
        WORKING_DIRECTORY "${checkoutDirectory}/${internalRepoBase}/${internalRepoName}/"
        COMMAND ${GIT_EXECUTABLE} checkout ${internalVersion} internal/
    )

    # Copy the contents of the public repo into the internal repo
    file(GLOB directoryEntries "${checkoutDirectory}/intel/gvk/*")
    file(COPY ${directoryEntries} DESTINATION "${checkoutDirectory}/${internalRepoBase}/${internalRepoName}/")
endif()
