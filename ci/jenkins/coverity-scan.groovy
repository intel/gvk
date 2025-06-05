
deps = load "${env.WORKSPACE}/ci/jenkins/download-deps.groovy"
utilities = load "${env.WORKSPACE}/ci/jenkins/utilities.groovy"

COVERITY_BINARY_URL = 'https://coverity.binary.placeholder.com/coverity'
COVERITY_SERVER_URL = 'https://coverity.server.placeholder.com/connect'

enum OS { Windows, Linux }

def withCoverity(Closure body) {

    if (isUnix()) {

        echo "Linux Coverity scans are not enabled"

        ////////////////////////////////////////////////////////////////////////////////
        // NOTE : The following is a bash script to install and configure Coverity on
        //  Linux.  GPA isn't shipping on Linux at this time, so Coverity scans aren't
        //  enabled.  This will need to be revisted if/when GPA needs to ship on Linux.
        ////////////////////////////////////////////////////////////////////////////////

        /*

        #!/bin/bash

        # Download enterprise Coverity license
        curl -u ${DEPENDENCY_SERVER_USERNAME}:${DEPENDENCY_SERVER_PASSWORD} -O '${COVERITY_BINARY_URL}/license.dat'
        sudo mkdir -p '/opt/coverity/software/'
        sudo mv 'license.dat' '/opt/coverity/software/license.dat'

        # Download, install, and configure cov-analysis tool
        curl -O '${COVERITY_BINARY_URL}/cov-analysis-linux64-2024.6.1.sh'
        sudo chmod +x cov-analysis-linux64-2024.6.1.sh
        sudo ./cov-analysis-linux64-2024.6.1.sh -q \
            --installation.dir=/opt/coverity/2024.6.1/analysis \
            --license.agreement=agree \
            --license.region=0 \
            --license.type.choice=0 \
            --license.cov.path=/opt/coverity/software/license.dat \
            --component.sdk=false \
            --component.skip.documentation=true
        sudo cp /opt/coverity/software/license.dat /opt/coverity/2024.6.1/analysis/bin/
        sudo /opt/coverity/2024.6.1/analysis/bin/cov-configure --gcc
        sudo /opt/coverity/2024.6.1/analysis/bin/cov-configure --compiler c++ --comptype g++ --template

        # Donwload, install, and configure cov-reports tool
        curl -O '${COVERITY_BINARY_URL}cov-reports-linux64-2024.6.1.sh'
        sudo chmod +x cov-reports-linux64-2024.6.1.sh
        sudo ./cov-reports-linux64-2024.6.1.sh -q --installation.dir=/opt/coverity/2024.6.1/reports/

        */
    } else {
        dir('coverity') {

            // Download Coverity license, cov-analysis, and cov-reports
            def url = '${COVERITY_BINARY_URL}'
            bat 'curl -O ' + url + 'license.dat'
            bat 'curl -O ' + url + 'cov-analysis-win64-2024.6.1.exe'
            bat 'curl -O ' + url + 'reports/cov-reports-win64-2024.6.1.exe'

            // Install and configure cov-analysis and cov-reports
            bat "cov-analysis-win64-2024.6.1.exe -q " +
                "--installation.dir=${env.WORKSPACE}\\coverity\\analysis\\ " +
                "--license.agreement=agree " +
                "--license.region=0 " +
                "--license.type.choice=0 " +
                "--license.cov.path=${env.WORKSPACE}\\coverity\\license.dat " +
                "--component.sdk=false " +
                "--component.skip.documentation=true"
            bat "copy license.dat ${env.WORKSPACE}\\coverity\\analysis\\bin\\"
            bat "cov-reports-win64-2024.6.1.exe -q --installation.dir=${env.WORKSPACE}\\coverity\\reports\\"
        }

        try {
            // Execute body with Coverity environment
            def custom_env = []
            custom_env.add("PATH+PATH=${env.WORKSPACE}\\coverity\\analysis\\bin\\;${env.WORKSPACE}\\coverity\\reports\\bin\\")
            withEnv(custom_env) {
                body()
            }
        } finally {
            // Uninstall cov-analysis and cov-reports
            bat "${env.WORKSPACE}\\coverity\\analysis\\uninstall.exe -q"
            bat "${env.WORKSPACE}\\coverity\\reports\\uninstall.exe -q"
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// NOTE : GPA isn't shipping on Linux at this time, so Coverity scans aren't
//  enabled.  This will need to be revisted if/when GPA needs to ship on Linux.
////////////////////////////////////////////////////////////////////////////////
// def coverity_scan_linux() {
//     gitlib.checkout(utilities.get_current_branch(), GIT_URL, GIT_CREDENTIALS)
//     def buildRoot = "ci/docker/"
//     def dockerFile = "Dockerfile-Ubuntu-24.04"
//     def coverityDir = "/opt/coverity/2024.6.1/analysis/"
//     def image = docker.build("gvk-build-u2404", "--build-arg PROXY_URL=${PROXY_URL} --build-arg NO_PROXY=${NO_PROXY} -f ${buildRoot}/${dockerFile} ${env.WORKSPACE}")
//     image.inside("-v $WORKSPACE:/build --mount type=bind,source=${coverityDir},target=${coverityDir}") {
//         def environment = []
//         environment.add("PATH+PATH=${coverityDir}/bin/")
//         withEnv(environment) {
//             sh 'cmake -B build -DCMAKE_BUILD_TYPE=Release -DPYTHON_EXECUTABLE:FILEPATH=${PYTHONPATH}/python.exe -C ci/build-options.cmake'
//             sh 'mkdir coverity-result'
//             sh 'cov-configure --gcc'
//             sh 'cov-build --dir coverity-result make -C build'
//             sh 'cat coverity-result/build-log.txt'
//             sh 'cov-analyze --dir coverity-result --concurrency --security --rule --enable-constraint-fpp --enable-fnptr --enable-virtual'
//             withCredentials([usernamePassword(credentialsId: AUTO_TEST_CREDENTIALS_ID, usernameVariable: 'COVERITY_USERNAME', passwordVariable: 'COVERITY_PASSWORD')]) {
//                 // NOTE : https://community.synopsys.com/s/article/Permission-denied-if-443-is-used-for-https-port-of-Coverity-Platform-on-Linux
//                 sh 'cov-commit-defects --dir coverity-result --url ${COVERITY_SERVER_URL} --stream GVK-Linux --user ${COVERITY_USERNAME} --password ${COVERITY_PASSWORD}'
//             }
//         }
//     }
// }

def coverity_scan_windows() {
    gitlib.checkout(utilities.get_current_branch(), GIT_URL, GIT_CREDENTIALS)
    environment.withCoverity({
        if (params.SCAN_AND_COMMIT) {
            def cmakeDir = "${env.WORKSPACE}\\" + deps.download_dependency('cmake', "${OS.Windows}")
            def pythonDir = "${env.WORKSPACE}\\" + deps.download_dependency('python', "${OS.Windows}")
            def msbuildDir = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\MSBuild\\Current\\Bin\\"
            def environment = []
            environment.add("PATH+PATH=${cmakeDir}\\bin\\;${pythonDir};${msbuildDir}")
            environment.add("PYTHONPATH=${pythonDir}")
            withEnv(environment) {
                sh 'cmake -G "Visual Studio 17 2022" -A x64 -B build -C ci/build-options.cmake -D PYTHON_EXECUTABLE:FILEPATH=${PYTHONPATH}/python.exe'
                sh 'mkdir coverity-result'
                sh 'cov-configure.exe --msvc'
                sh 'cov-build.exe --dir coverity-result msbuild.exe build/gvk.sln /property:Configuration=Release'
                sh 'cov-analyze.exe --dir coverity-result --concurrency --security --rule --enable-constraint-fpp --enable-fnptr --enable-virtual'
                withCredentials([usernamePassword(credentialsId: AUTO_TEST_CREDENTIALS_ID, usernameVariable: 'COVERITY_USERNAME', passwordVariable: 'COVERITY_PASSWORD')]) {
                    sh 'cov-commit-defects.exe --dir coverity-result --url ${COVERITY_SERVER_URL} --stream GVK-Windows --user ${COVERITY_USERNAME} --password ${COVERITY_PASSWORD}'
                }
            }
        }
        if (params.GENERATE_REPORT) {
            withCredentials([usernamePassword(credentialsId: AUTO_TEST_CREDENTIALS_ID, usernameVariable: 'COVERITY_USERNAME', passwordVariable: 'COVERITY_PASSWORD')]) {
                sh 'export COVERITY_PASSWORD=${COVERITY_PASSWORD}'
                sh 'cov-generate-cvss-report.exe ci/jenkins/coverity-report.yaml --report --output gvk-coverity-cvss-report.pdf --user ${COVERITY_USERNAME} --password env:COVERITY_PASSWORD'
                archiveArtifacts artifacts: "gvk-coverity-cvss-report.pdf"
                sh 'cov-generate-security-report.exe ci/jenkins/coverity-report.yaml --output gvk-coverity-security-report.pdf --user ${COVERITY_USERNAME} --password env:COVERITY_PASSWORD'
                archiveArtifacts artifacts: "gvk-coverity-security-report.pdf"
            }
        }
    })
}

pipeline {
    agent none
    parameters {
        booleanParam(name: "SCAN_AND_COMMIT", defaultValue: true, description: "Whether or not to execute and commit Coverity scan")
        booleanParam(name: "GENERATE_REPORT", defaultValue: true, description: "Whether or not to generate Coverity reports")
    }
    stages {
        stage('Coverity') {
            parallel {
                ////////////////////////////////////////////////////////////////////////////////
                // NOTE : GPA isn't shipping on Linux at this time, so Coverity scans aren't
                //  enabled.  This will need to be revisted if/when GPA needs to ship on Linux.
                ////////////////////////////////////////////////////////////////////////////////
                // stage('Linux') {
                //     agent { label "${UBUNTU_24_04_BUILDER}" }
                //     steps { script {
                //         utilities.start_stage()
                //         coverity_scan_linux()
                //     }}
                // }
                stage('Windows') {
                    agent { label "${WINDOWS_BUILDER}" }
                    steps { script {
                        utilities.start_stage()
                        coverity_scan_windows()
                    }}
                }
            }
        }
    }
}
