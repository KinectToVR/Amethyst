FROM mcr.microsoft.com/windows/servercore:ltsc2022-amd64 AS main
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

# Prepare the environment : kienct sdks
RUN powershell.exe -Command \
    Set-ExecutionPolicy Bypass -Scope Process -Force; \
    Invoke-Expression (Get-Content .dockerkinect -Raw)

# Prepare the environment
RUN powershell.exe -Command \
    Set-ExecutionPolicy Bypass -Scope Process -Force; \
    Invoke-Expression (Get-Content .dockerprep -Raw)

# Download dependencies
RUN powershell.exe -Command \
    Set-ExecutionPolicy Bypass -Scope Process -Force; \
    Invoke-Expression (Get-Content .dockerdeps -Raw)

# Build the solution
RUN powershell.exe -Command \
    Set-ExecutionPolicy Bypass -Scope Process -Force; \
    Invoke-Expression (Get-Content .dockerbuild -Raw)