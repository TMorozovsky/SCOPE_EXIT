cd ..
mkdir build
cd build
conan install .. --build=missing
cmake ../scope_exit_tests
cd ..
cd build_scripts

