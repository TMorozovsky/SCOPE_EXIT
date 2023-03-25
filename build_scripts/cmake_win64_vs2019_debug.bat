cd ..
mkdir build_debug
cd build_debug
conan install .. -s build_type=Debug -s compiler="Visual Studio" -s compiler.version=16 -s compiler.runtime=MDd --build=missing
cmake.exe -G "Visual Studio 16 2019" -A x64 ../scope_exit_tests -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CONFIGURATION_TYPES=Debug
cd ..
cd build_scripts

