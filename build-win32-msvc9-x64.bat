if not exist build mkdir build
cd build
if not exist win32-msvc9-x64 mkdir win32-msvc9-x64
cd win32-msvc9-x64
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x64
CMakeSetup.exe ..\..
if exist CRAlgo.sln start /MAX Devenv Lcpp.sln
cd ../..
