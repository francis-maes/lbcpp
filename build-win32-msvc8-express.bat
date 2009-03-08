cd build
mkdir win32-msvc8-express
cd win32-msvc8-express
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat"
CMakeSetup.exe ..\..
start /MAX VCExpress CRAlgo.sln
cd ../..
