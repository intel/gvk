
--------------------------------------------------------------------------------
### Build Linux

#### Quick Start
- If your development environment is already setup
- Using Bash from the GVK root directory
> `time cmake -B build`  
> `time cmake --build build --target install`  
> `./build/samples/gvk-getting-started-00-triangle`  
    - Note that the `time` command isn't necessary, but is added for conveneience

#### Detailed Instructions
- [Update APT (Advanced Package Tool)](#Update-APT)
- [Install Build Tools](#Install-Build-Tools)
- [Setup Git SSH](#Setup-Git-SSH)
- [Clone Repository](#Clone-Repository)
- [Configure and Build](#Configure-and-Build)
- [Configure Vulkan SDK](#Configure-Vulkan-SDK)

#### Tips & Tricks
- [SSH Access from Windows](#SSH-Access-from-Windows)
- [Notepad++ FTP (NppFTP)](#NppFTP)
- [Remote Desktop Access from Windows](#Remote-Desktop-Access-from-Windows)

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
## Detailed Instructions
--------------------------------------------------------------------------------
### Update APT
- Update available packages, upgrade installed packages, remove unused packages
> `sudo apt update`  
> `sudo apt upgrade`  
> `sudo apt autoremove`

--------------------------------------------------------------------------------
### Install Build Tools
- Install build tools
> `sudo apt install build-essential`  
> `sudo apt install cmake`  
> `sudo apt install git`  
> `sudo apt install libwayland-dev`  
> `sudo apt install libxkbcommon-dev`  
> `sudo apt install python3`  
> `sudo apt install xorg-dev`
- Configure GCC/G++ as the default C/C++ compilers for the current environment
> `export CC=/usr/bin/gcc`  
> `export CXX=/usr/bin/g++`
- Configure GCC/G++ to be set as the default C/C++ compilers whenever a new environment is created (ie. on starup or when openening a new terminal)
> `echo "export CC=/usr/bin/gcc" >> ~/.profile`  
> `echo "export CXX=/usr/bin/g++" >> ~/.profile`

--------------------------------------------------------------------------------
### Setup Git SSH
- Generate an SSH key
> `ssh-keygen -o`
- You can press [Enter] at the directory prompt to use the default directory.  
FROM : https://git-scm.com/book/ms/v2/Git-on-the-Server-Generating-Your-SSH-Public-Key  
*"First it confirms where you want to save the key (`.ssh/id_<alg>`), and then it asks twice for a passphrase, which you can leave empty if you don't want to type a password when you use the key. However, if you do use a password, make sure to add the `-o` option; it saves the private key in a format that is more resistant to brute-force password cracking than is the default format. You can also use the ssh-agent tool to prevent having to enter the password each time."*
- Output the public key contents  
*Note that `<alg>` should be replaced with the SSH algorithm used, you can use `ls -la ~/.ssh` to see the name of the file*
> `cat ~/.ssh/id_<alg>.pub`
- Output should be similar to `ssh-<alg> <ssh-key> <user>@<machine-name>`
- Sign into GitHub in your browser, goto **Settings > SSH and GPG keys**
- Click **[New SSH key]**
- Fill out the title field with a memorable name
- Copy/paste the output of the `cat` command above into the **Key** field
- Click **[Configure SSO]** and select any private organization you're authorized to access, and you'd like to access via this SSH key
- Configure Git to access repositories using SSH instead of HTTPS
> `git config --global url."git@github.com:".insteadOf "https://github.com/"`

--------------------------------------------------------------------------------
### Clone Repository
- Create a "~/intel" directory and clone GVK there, feel free to use a different directory if desired
> `mkdir ~/intel`  
> `cd ~/intel`  
> `git clone git@github.com:intel/gvk.git`

--------------------------------------------------------------------------------
### Configure and Build
- Using Bash from the GVK root directory
> `time cmake -B build`  
> `time cmake --build build --target install`  
> `./build/samples/gvk-getting-started-00-triangle`  
    - Note that the `time` command isn't necessary, but is added for conveneience

--------------------------------------------------------------------------------
### Configure Vulkan SDK
- GVK will download the required Vulkan SDK at configure time if necessary; see GVK's root CMakeLists for the required version
- The GVK build does not install the Vulkan SDK, set environment variables, or system path
- To set these after GVK configuration
> `source build/_deps/VulkanSDK/<version>/setup-env.sh`
- Alternatively, the downloaded Vulkan SDK can be used directly by setting the environment variable `VULKAN_SDK` to point to `gvk/build/_deps/VulkanSDK/<version>/`
    - For more info on configuring layers see
        - https://github.com/KhronosGroup/Vulkan-Loader/blob/main/docs/LoaderLayerInterface.md
- For advanced users configuring automated environments the following scripts are provided
    - For downloading and extracting the required Vulkan SDK
        - `gvk/build/cmake/gvk-vulkan-sdk.cmake`
        - `gvk/install/cmake/gvk-vulkan-sdk.cmake`

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
## Tips & Tricks
--------------------------------------------------------------------------------
### SSH Access from Windows
- Using Git Bash/MinGW
> `ssh <username>@<ip-address>`  
> `<username>@<ip-address>'s password: <password>`
- You are now connected to the remote machine, you can sign out with [Ctrl]+[D], `exit`, or `logout`

--------------------------------------------------------------------------------
### NppFTP
- Update Notepad++ from the main menu select **? > Update Notepad++**
- Close Notepad++
- From the NppFTP GitHub page https://github.com/ashkulz/NppFTP
- Click [Tags], then on the latest tagged release click [Downloads]
- Download `NppFTP-x64.zip` (or `NppFTP-x86.zip` if using 32 bit Notepad++)
- Extract the zip
- Run Notepad++ as administrator
- Import NppFTP from the main menu **Settings > Import > Import plugin(s)**
- Navigate to the extracted directory and select `NppFTP.dll`
- Restart Notepad++ (without administrator privileges)
- From the main menu select **Plugins > NppFTP > Show NppFTP Window**
- From the NppFTP menu select **Settings > Profile Settings**
- Click [Add new]
- Fill out the name field with a memorable name
- Fill out **Hostname** with the machine's ip address
- Fill out **Username** with the remote machine usrname
- Set Connection type to SFTP
- Check "Ask for Password"
- Click [Close]
- Right click on the connection profile and select Connect
- Enter the remote machine password
- Click [OK]
- You are now connected to the remote machine, you can disconnect by shutting down the machine or using the first button of the NppFTP menu [(Dis)Connect]

--------------------------------------------------------------------------------
### Remote Desktop Access from Windows
- Sign in via SSH
- Install and configure XFCE; Select **`lightdm`** and **`xfce4-session`** when prompted
> `sudo apt update`  
> `sudo apt upgrade`  
> `sudo apt autoremove`  
> `sudo apt install xfce4-goodies xfce4`  
> `sudo update-alternatives --config x-session-manager`  
> `sudo reboot`
- Wait a few minutes, then sign in via SSH again
- Install XRDP and confirm its status
> `sudo apt install xrdp`  
> `sudo systemctl status xrdp`
- Output should have **`Active: active (running)`** in the first few lines
- Now start Remote Desktop Connection on your Windows machine with
    - `<ip-address>:3389` (3389 is the port RDP uses by default)
- Sign in using the remote machine's credentials
- You are now connected to the remote machine, you can disconnect by shutting down the machine or by ending your RDP session
