@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build

