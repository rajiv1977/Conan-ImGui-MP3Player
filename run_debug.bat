@ECHO ON

set BASEDIR=%~dp0
PUSHD %BASEDIR%

RMDIR /Q /S build


conan install . -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True --output-folder=build --build=missing --settings=build_type=Debug
cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=./build/generators/conan_toolchain.cmake -DCMAKE_POLICY_DEFAULT_CMP0091=NEW 
cmake --build . --config Debug
robocopy ../assets/visualizer/webfonts Debug fa-brands-400.ttf /z
robocopy ../assets/visualizer/webfonts Debug fa-regular-400.ttf /z
robocopy ../assets/visualizer/webfonts Debug fa-solid-900.ttf /z
robocopy ../assets/visualizer/webfonts Debug fa-v4compatibility.ttf /z
robocopy ../test Debug Oryza.mp3 /z
Debug\MP3PLAYER.exe 