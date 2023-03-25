cmake_minimum_required(VERSION 3.25)

project(scope_exit_tests)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# Find GTest

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(GTest REQUIRED)


# Add a unit tests project

add_executable(scope_exit_tests
               test_scope_exit.cpp)

target_include_directories(scope_exit_tests PRIVATE "..")

if (${NO_CPP20_CONSTEXPR_DESTRUCTORS})
  set_property(TARGET scope_exit_tests PROPERTY CXX_STANDARD 17)
else()
  set_property(TARGET scope_exit_tests PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(scope_exit_tests PRIVATE GTest::gtest_main)

add_custom_command(TARGET scope_exit_tests
                   POST_BUILD
                   COMMAND "$<TARGET_FILE:scope_exit_tests>")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT scope_exit_tests)


# Visual Studio-specific: add the library itself as a target in order to be more IDE-friendly
if (CMAKE_GENERATOR MATCHES "Visual Studio")
  get_filename_component(SCOPE_EXIT_FILES_ABSOLUTE_PATH "../scope_exit" ABSOLUTE)

  file(GLOB_RECURSE SCOPE_EXIT_ALL_FILES
       LIST_DIRECTORIES FALSE
       "${SCOPE_EXIT_FILES_ABSOLUTE_PATH}/*")

  foreach(SCOPE_EXIT_FILE IN ITEMS ${SCOPE_EXIT_ALL_FILES})
      get_filename_component(SCOPE_EXIT_FILE_PATH "${SCOPE_EXIT_FILE}" PATH)
      file(RELATIVE_PATH SCOPE_EXIT_FILE_RELATIVE_PATH "${SCOPE_EXIT_FILES_ABSOLUTE_PATH}" "${SCOPE_EXIT_FILE_PATH}")
      string(REPLACE "/" "\\" SCOPE_EXIT_FILE_GROUP_PATH "${SCOPE_EXIT_FILE_RELATIVE_PATH}")
      source_group("${SCOPE_EXIT_FILE_GROUP_PATH}" FILES "${SCOPE_EXIT_FILE}")
  endforeach()

  add_library(scope_exit INTERFACE ${SCOPE_EXIT_ALL_FILES})

  if (${NO_CPP20_CONSTEXPR_DESTRUCTORS})
    set_property(TARGET scope_exit PROPERTY CXX_STANDARD 17)
  else()
    set_property(TARGET scope_exit PROPERTY CXX_STANDARD 20)
  endif()
endif()
