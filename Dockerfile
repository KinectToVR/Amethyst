FROM mcr.microsoft.com/windows/servercore:ltsc2019-amd64 AS main
LABEL Description="IIS" Vendor="Microsoft" Version="10"
ADD ./ /

# Prepare the environment : vctools
RUN powershell.exe -Command \
    Set-ExecutionPolicy Bypass -Scope Process -Force; \
    Invoke-Expression (Get-Content .dockerprepvc -Raw)

# Prepare the environment : chocolatey
RUN powershell.exe -Command \
    Set-ExecutionPolicy Bypass -Scope Process -Force; \
    Invoke-Expression (Get-Content .dockerprepchoco -Raw)

# Prepare the environment
RUN powershell.exe -Command \
    Set-ExecutionPolicy Bypass -Scope Process -Force; \
    Invoke-Expression (Get-Content .dockerprep_nolink -Raw); \
    powershell.exe -Command "&"C:/vcpkg/vcpkg.exe" install kinectsdk1:x64-windows kinectsdk2:x64-windows"; ` \
    sed -e 's/Kinect10.lib;//g' -i "device_KinectV1/device_KinectV1.vcxproj"; ` \
    sed -e 's/Kinect20.lib;//g' -i "device_KinectV2/device_KinectV2.vcxproj"

# Download dependencies
RUN powershell.exe -Command \
    Set-ExecutionPolicy Bypass -Scope Process -Force; \
    Invoke-Expression (Get-Content .dockerdeps -Raw)

# Build the solution
RUN powershell.exe -Command \
    Set-ExecutionPolicy Bypass -Scope Process -Force; \
    Invoke-Expression (Get-Content .dockerbuild -Raw)