@echo off

if not exist "vcpkg" (
    git clone https://github.com/microsoft/vcpkg.git
)

cd vcpkg

git reset --hard fb544875b93bffebe96c6f720000003234cfba08

if not exist "vcpkg.exe" (
    bootstrap-vcpkg.bat
)

