cd ..
mkdir build_cpp17_mode
cd build_cpp17_mode
conan install .. --build=missing
cmake ../scope_exit_tests -DNO_CPP20_CONSTEXPR_DESTRUCTORS=1
cd ..
cd build_scripts
