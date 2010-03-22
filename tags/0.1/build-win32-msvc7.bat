cd build
mkdir win32-msvc7
cd win32-msvc7
call "C:\Program Files\Microsoft Visual Studio .NET\Vc7\bin\vcvars32.bat"
CMakeSetup.exe ..\..
start /MAX devenv.exe LBCpp.sln
cd ../..
