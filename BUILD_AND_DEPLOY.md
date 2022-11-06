## **Build Amethyst**
Precautions:
 - If you've <32GB RAM, don't even try (or create a giant pagefile)
 - Don't build Amethyst on `Debug` configuration (it's gonna crash)
 - Building on SSD is about 5-8x faster than on HDD!

You'll need:
 - The ```Amethyst``` repo cloned somewhere and ```cd```'d into
 - (For building plugins) Kinect SDK 1.8 & 2.0 installed and in PATH
 - (For testing purpose) Working installation of SteamVR

Follow these steps:

- [Install tools for the Windows App development](https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/set-up-your-development-environment?tabs=vs-2022-17-1-a%2Cvs-2022-17-1-b).<br>
  You'll have to install Visual Studio 2022, [WASDK Plugins and Runtime](https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads).<br>
  (Make sure to check MFC, UWP, C++/CLI, .NET Desktop & Framework<br>
  and proper v143 Tools & Windows SDK when installing Visual Studio)

- Install `vcpkg` and its Visual Studio integration<br>
  (If you already have `vcpkg` set up, **please only install libraries**)<br>
  ```powershell
  # WARNING: DO THIS ONLY IF YOU DON'T HAVE VCPKG ON YOUR PC
  # IMPORTANT: First cd into some appropriate dir, e.g. C:/
  > git clone https://github.com/Microsoft/vcpkg.git
  > cd ./vcpkg
  > ./bootstrap-vcpkg.bat
  ```
  When vcpkg is set up, intgrate it and install the needed libraries:
  ```powershell
  > ./vcpkg integrate install
  > ./vcpkg install `
    eigen3:x64-windows-static `
    glog:x64-windows-static `
    gflags:x64-windows-static `
    protobuf:x64-windows-static `
    cereal:x64-windows-static `
    grpc:x64-windows-static
  ```

- Clone the latest `OpenVR` to `external/`<br>
  ```powershell
  # Set up external dependencies
  > git submodule update --init
  ```

- Build Amethyst:<br>
  - Option 1:
    Open `Amethyst.sln` in Visual Studio and `Build Solution`
  - Option 2, via CLI:
  ```powershell
  # Download the vswhere tool to find msbuild without additional interactions
  > Invoke-WebRequest -Uri 'https://github.com/microsoft/vswhere/releases/latest/download/vswhere.exe' -OutFile './vswhere.exe'
  # Use the downloaded vswhere tool (again) to find msbuild. Skip this step if you use the Dev Powershell
  > $msbuild = "$("$(.\vswhere.exe -legacy -prerelease -products * -format json | Select-String -Pattern "2022" | `
    Select-String -Pattern "Studio" | Select-Object -First 1 | Select-String -Pattern "installationPath")" `
    -replace('"installationPath": "','') -replace('",',''))".Trim() + "\\MSBuild\\Current\\Bin\\MSBuild.exe"

  # Restore NuGet packages and build everything. Remove the /m:3 if your PC is kinda slow (or just give up)
  > &"$msbuild" Amethyst.sln /t:restore "/p:Configuration=Release;Platform=x64;RestorePackagesConfig=true"
  > &"$msbuild" Amethyst.sln /m:3 "/p:Configuration=Release;Platform=x64;BuildInParallel=true"
  ```

## **Deployment**
The whole output can be found in ```x64/Release``` directory<br>
(or ```x64/Debug``` if you're building for ```Debug```, naturally) and:
 - The assembled ```K2Driver``` is inside the ```driver/``` folder
 - Devices (plugins) are inside ```devices/``` folder
 - Deployed app is inside ```Amethyst/``` folder, along with devices (plugins)<br>
 - To run the deployed app you need [VC2022 Redist](https://aka.ms/vs/17/release/vc_redist.x64.exe), [.NET6 Redist](https://download.visualstudio.microsoft.com/download/pr/7f3a766e-9516-4579-aaf2-2b150caa465c/d57665f880cdcce816b278a944092965/windowsdesktop-runtime-6.0.3-win-x64.exe
) and [WASDK Runtime](https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads).
 - **Note:** you can't interface a ```Release``` devices with a ```Debug``` app! (driver yes tho)