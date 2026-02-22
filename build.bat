@echo off

set CMAKE_PREFIX_PATH=C:\Qt\6.10.2\msvc2022_64
set BUILD_TYPE=Release
if "%1"=="debug" set BUILD_TYPE=Debug

IF NOT EXIST build mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config %BUILD_TYPE%
cd ..
