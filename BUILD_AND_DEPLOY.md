## **Build Amethyst (K2App)**
You'll need:
 - The ```K2App``` repo cloned somewhere and ```cd```'d into
 - (For building plugins) Kinect SDK 1.8 & 2.0 installed and in PATH
 - (For testing purpose) Working installation of SteamVR

Follow these steps:

- [Install tools for the Windows App development](https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/set-up-your-development-environment?tabs=vs-2022-17-1-a%2Cvs-2022-17-1-b).<br>
  You'll have to install Visual Studio 2022, [WASDK Plugins and Runtime](https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads).

- Install [CMake](https://cmake.org/download/) and [git](https://git-scm.com/download/win) if you still somehow don't have them.<br>
  Note: If you have `chocolatey` installed, you can just ```choco install cmake git```

- Install `vcpkg` and its Visual Studio integration<br>
  (If you already have `vcpkg` set up, **please only install libraries**)<br>
  ```powershell
  # WARNING: DO THIS ONLY IF YOU DON'T HAVE VCPKG ON YOUR PC
  # IMPORTANT: First cd into some appropriate dir, e.g. C:/
  > git clone https://github.com/Microsoft/vcpkg.git
  > cd ./vcpkg
  > ./bootstrap-vcpkg.bat
  ```
  When vcpkg is set up, intgrate it and install some of `boost` and `curlpp`:
  ```powershell
  > ./vcpkg integrate install
  > ./vcpkg install `
    boost-serialization:x64-windows `
    boost-assign:x64-windows `
    boost-filesystem:x64-windows `
    boost-dll:x64-windows `
    boost-property-tree:x64-windows `
    boost-foreach:x64-windows `
    boost-lexical-cast:x64-windows `
    boost-unordered:x64-windows `
    boost-math:x64-windows `
    boost-algorithm:x64-windows `
    curlpp:x64-windows
  ```

- Clone latest `OpenVR` and `Eigen3` to `external/` and set up `GLog` & `GFlags`<br>
  (For this step you must have cmake installed and visible in PATH)<br>
  ```powershell
  # Download the vswhere tool to find msbuild without additional interactions
  > Invoke-WebRequest -Uri 'https://github.com/microsoft/vswhere/releases/latest/download/vswhere.exe' -OutFile './vswhere.exe'
  # Use the downloaded vswhere tool to find msbuild. Skip this step if you use the Dev Powershell
  > $msbuild = "$("$(.\vswhere.exe -legacy -prerelease -products * -format json | Select-String -Pattern "2022" | `
        Select-String -Pattern "Studio" | Select-Object -First 1 | Select-String -Pattern "installationPath")" `
        -replace('"installationPath": "','') -replace('",',''))".Trim() + "\\MSBuild\\Current\\Bin\\MSBuild.exe"

  # Clone OpenVR and Eigen3 libraries
  > git clone https://github.com/ValveSoftware/openvr ./external/openvr
  > git clone https://gitlab.com/libeigen/eigen ./external/eigen
  # Reset Eigen to the latest OK state
  > cd ./external/eigen
  > git reset --hard 1fd5ce1002a6f30e1169b529b291216a18be2f7e
  # Go back
  > cd ../..

  # Clone and setup GLog
  > git clone https://github.com/google/glog.git ./external/glog
  # Reset GLog to the latest OK state and build it
  > cd ./external/glog
  > git reset --hard f8c8e99fdfb998c2ba96cfb470decccf418f0b30
  > mkdir vcbuild; cd ./vcbuild
  > cmake .. -DBUILD_SHARED_LIBS=ON
  > &"$msbuild" glog.vcxproj "/p:Configuration=Release;Platform=x64"
  # Go back
  > cd ../../..

  # Clone and setup GFlags
  > git clone https://github.com/gflags/gflags.git ./external/gflags
  # Reset GFlags to the latest OK state and build it
  > cd ./external/gflags
  > git reset --hard 827c769e5fc98e0f2a34c47cef953cc6328abced
  > mkdir vcbuild; cd ./vcbuild
  > cmake .. -DBUILD_SHARED_LIBS=ON
  > &"$msbuild" gflags.vcxproj "/p:Configuration=Release;Platform=x64"
  # Go back
  > cd ../../..
  ```

- Build the K2App:<br>
  ```powershell
  # Use the downloaded vswhere tool (again) to find msbuild. Skip this step if you use the Dev Powershell
  > $msbuild = "$("$(.\vswhere.exe -legacy -prerelease -products * -format json | Select-String -Pattern "2022" | `
    Select-String -Pattern "Studio" | Select-Object -First 1 | Select-String -Pattern "installationPath")" `
    -replace('"installationPath": "','') -replace('",',''))".Trim() + "\\MSBuild\\Current\\Bin\\MSBuild.exe"

  # Restore NuGet packages and build everything. Remove the /m:3 if your PC is kinda slow (or just give up)
  > &"$msbuild" KinectToVR.sln /t:restore "/p:Configuration=Release;Platform=x64;RestorePackagesConfig=true"
  > &"$msbuild" KinectToVR.sln /m:3 "/p:Configuration=Release;Platform=x64;BuildInParallel=true"
  ```

## **Deployment**
The whole output can be found in ```x64/Release``` directory<br>
(or ```x64/Debug``` if you're building for ```Debug```, naturally) and:
 - The assembled ```K2Driver``` is inside the ```driver/``` folder
 - Devices (plugins) are inside ```devices/``` folder
 - Deployed app is inside ```KinectToVR/``` folder, along with devices (plugins)<br>
 - To run the deployed app you need [VC2022 Redist](https://aka.ms/vs/17/release/vc_redist.x64.exe), [.NET6 Redist](https://download.visualstudio.microsoft.com/download/pr/7f3a766e-9516-4579-aaf2-2b150caa465c/d57665f880cdcce816b278a944092965/windowsdesktop-runtime-6.0.3-win-x64.exe
) and [WASDK Runtime](https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads).
 - **Note:** you can't interface a ```Release``` devices with a ```Debug``` app! (driver yes tho)