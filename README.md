<h1 dir=auto>
<b>K2App</b>
<text>(&nbspCodename</text>
<text style="color:#9966cc;">Amethyst</text>
<text>)</text>
</h1>

## <ins>__[Discord server](https://discord.gg/YBQCRDG)__</ins> and I'm **公彦赤屋先#5023**

## **License**
This project is licensed under the GNU GPL v3 License 

## **Build with Docker**
You'll need:
 - The ```K2App``` repo cloned somewhere and ```cd```'d into
 - Docker Desktop set to use Windows containers

Notes:
 - The build process contains no caching, is one-run, **and will take some time**.<br>
   Everything that's downloaded, set up, and not saved will be gone.

Follow these steps:
  ```powershell
  # Build the whole thing
  > docker build -t k2app .

  # Create a dummy container from saved image
  > $id = $(docker create k2app)
  # Copy Release files to the host machine
  > mkdir ./x64/Release
  > docker cp ${id}:/x64/Release ./x64/Release

  # Release (remove) the dummy container
  > docker rm -v $id
  # Release (remove) the base k2app container
  > docker rmi --force $(docker images -q 'k2app')
  ```
  Where ```k2app``` may be your very own super cool container tag name.<br>
  Artifacts (```x64/Release``` directory) will be saved to ```x64/Release``` inside the repository root.<br>

  
## **Build manually**
You'll need:
 - The ```K2App``` repo cloned somewhere and ```cd```'d into
 - Kinect SDK 1.8 & 2.0 installed and visible in PATH
 - Working installation of SteamVR for testing

Follow these steps:

- Install Visual Studio 2022 Build Tools with needed addons<br>
  (If you already have VS2022 or BuildTools set up, **please skip this step**)<br>
  ```powershell
  > Set-ExecutionPolicy Bypass -Scope Process -Force
  > Invoke-Expression (Get-Content .dockerprepvc -Raw)
  ```
  This will install Microsoft Visual Studio Build Tools 2022 in the default location.<br>
  Nothing will be displayed during the process (just the PID & stats), just wait for it.

- Install ```chocolatey``` and ```cmake```, ```7zip.install```, ```sed```, ```git```<br>
  (If you already have ```chocolatey``` set up, **please only install packages**)<br>
  ```powershell
  > Set-ExecutionPolicy Bypass -Scope Process -Force
  > Invoke-Expression (Get-Content .dockerprepchoco -Raw)
  ```
  This will install ```chocolatey``` and download all needed libraries.<br>
  If you had vcpkg already set up, install ```cmake```, ```7zip.install```, ```sed``` and ```git```:
  ```powershell
  > choco install -y cmake 7zip.install sed git
  ```

- Install ```vcpkg``` and its Visual Studio integration<br>
  (If you already have ```vcpkg``` set up, **please only install libraries**)<br>
  ```powershell
  > Set-ExecutionPolicy Bypass -Scope Process -Force
  > Invoke-Expression (Get-Content .dockerprep -Raw)
  ```
  This will install ```vcpkg``` in ```C:/vcpkg``` and download all needed libraries.
  If you had vcpkg already set up, install ```boost``` and ```curlpp```:
  ```powershell
  > vcpkg install boost:x64-windows curlpp:x64-windows
  ```

- Clone latest OpenVR and Eigen3 to ```external/``` and set up ```GLog``` & ```GFlags```<br>
  (For this step you must have cmake installed and visible in ```C:/Program Files/CMake/bin/cmake.exe```)<br>
  Skip this step **only** when you know what you're setting things up some other way.
  ```powershell
  > Set-ExecutionPolicy Bypass -Scope Process -Force
  > Invoke-Expression (Get-Content .dockerdeps -Raw)
  ```

- Build the K2App:<br>
  ```powershell
  > Set-ExecutionPolicy Bypass -Scope Process -Force
  > Invoke-Expression (Get-Content .dockerbuild -Raw)
  ```
  Alternatively you can open the developer powershell for Visual Studio (or BuildTools) 2022 and:
  ```powershell
  > msbuild KinectToVR.sln /t:restore "/p:Configuration=Release;Platform=x64;RestorePackagesConfig=true"
  > msbuild KinectToVR.sln /m:3 "/p:Configuration=Release;Platform=x64;BuildInParallel=true"
  ```
  There are 2 commands since the first will only restore ```NuGet```,  then the whole thing will be built.

## **Deploy**
The whole output can be found in ```x64/Release``` directory<br>
(or ```x64/Debug``` if you're building for ```Debug```, naturally) and:
 - The assembled ```K2Driver``` is inside the ```driver/``` folder
 - Devices (plugins) are inside ```devices/``` folder
 - Deployed app is inside ```KinectToVR/``` folder, along with devices (plugins)<br>
 - To run the deployed app you'll need to install the [Windows App Runtime](https://aka.ms/windowsappsdk/1.0-stable/msix-installer).
 - **Note:** you can't interface a ```Release``` driver with a ```Debug``` app!