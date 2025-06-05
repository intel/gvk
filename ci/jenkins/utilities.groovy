
AUTO_TEST_USER = "dmn\\placeholder"
AUTO_TEST_CREDENTIALS_ID = 'placeholder'
WINDOWS_BUILDER = '(win10 || win11) && builder'
WINDOWS_TESTER = '(win10 || win11) && tester'
UBUNTU_24_04_BUILDER = 'u2404 && builder && docker'
UBUNTU_24_04_TESTER = 'u2404 && tester'
GIT_CREDENTIALS = 'placeholder'
GIT_URL = 'https://github.com/intel/gvk.git'
PROXY_URL='http://proxy.placeholder.com:8080'
NO_PROXY='placeholder.com'

def powershell(psCmd) {
    bat "powershell.exe -ExecutionPolicy Bypass -File ${psCmd}"
}

def start_stage() {
    echo " ${env.STAGE_NAME} > ${NODE_NAME}"
    cleanWs()
}

def report_failure(Exception e) {
    gitlib.report_failure_commit_status("Build #${env.BUILD_NUMBER}")
    echo 'Failed with exception: \n' + e.toString()
    throw e
}

def get_current_branch() {
    // applications.analyzers.gpa.backend.gvk
    def current_branch = gitlib.get_current_branch()
    if (current_branch == null) {
        current_branch = "trunk"
    }
    return current_branch
}

def get_stash_name(stashName) {
    return stashName + "-${env.STAGE_NAME}".replaceAll(' ', '-')
}

return this
