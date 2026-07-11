
@echo off
set "build_dir=Build"

echo Entering build.bat

if not exist %build_dir% mkdir %build_dir%

cl /nologo /Z7 /EHsc /std:c++17^
    "WinMain.cpp" "MessageMap.cpp" ^
    /Fo"%build_dir%\\" /Fd"%build_dir%\\" /Fe"%build_dir%\main.exe"
