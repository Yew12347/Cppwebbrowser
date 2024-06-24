@echo off

REM Clean previous build
rmdir /s /q build
mkdir build
cd build

REM Run qmake
qmake ..\my_project.pro

REM Build the project using nmake (for Visual Studio)
nmake
