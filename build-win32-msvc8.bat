cd build
mkdir win32-msvc8
cd win32-msvc8
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat"
CMakeSetup.exe ..\..
start /MAX Devenv Lcpp.sln
cd ../..
