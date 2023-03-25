cd ..
mkdir build_release_cpp17_mode
cd build_release_cpp17_mode
conan install .. -s build_type=Release -s compiler="Visual Studio" -s compiler.version=16 -s compiler.runtime=MD --build=missing
cmake.exe -G "Visual Studio 16 2019" -A x64 ../scope_exit_tests -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release -DNO_CPP20_CONSTEXPR_DESTRUCTORS=1
cd ..
cd build_scripts

