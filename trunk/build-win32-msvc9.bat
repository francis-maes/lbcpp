if not exist build mkdir build
cd build
if not exist win32-msvc9 mkdir win32-msvc9
cd win32-msvc9
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
CMakeSetup.exe ..\..
if exist CRAlgo.sln start /MAX Devenv CRAlgo.sln
cd ../..
