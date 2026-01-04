cmake_minimum_required(VERSION 3.14)
project(vray_demo CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()

# Prefer building bundled raylib if available under third_party/raylib
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/raylib/CMakeLists.txt")
  add_subdirectory(third_party/raylib)
  set(RAYLIB_TARGET raylib)
else()
  find_package(raylib QUIET)
  if(TARGET raylib)
    set(RAYLIB_TARGET raylib)
  else()
    message(WARNING "raylib not found. Please clone raylib into third_party/raylib or install it on your system.")
  endif()
endif()

file(GLOB_RECURSE VRAY_SOURCES "src/*.cpp")
list(FILTER VRAY_SOURCES EXCLUDE REGEX "src/test\\.cpp")

add_executable(vray_demo ${VRAY_SOURCES})

add_executable(smoke_tests
  tests/smoke_tests.cpp
  src/platform/platform.cpp
  src/platform/raylib_window.cpp
  src/platform/raylib_input.cpp
  src/platform/raylib_renderer.cpp
)

# Lightweight mesh sanity tests
add_executable(mesh_tests
  src/test.cpp
  src/utils/meshGenerateUtils.cpp
  src/utils/meshMathUtils.cpp
  src/utils/meshProcessUtils.cpp
  src/utils/meshExtraShapeUtils.cpp
  src/utils/meshCompositeUtils.cpp
)

# Shared include path and defines
target_include_directories(mesh_tests PRIVATE src)
target_compile_definitions(mesh_tests PRIVATE _CRT_SECURE_NO_WARNINGS)

target_include_directories(smoke_tests PRIVATE src)
target_compile_definitions(smoke_tests PRIVATE _CRT_SECURE_NO_WARNINGS)

# Set the root source directory for includes
target_include_directories(vray_demo PRIVATE src)

target_compile_definitions(vray_demo PRIVATE _CRT_SECURE_NO_WARNINGS)


# If raygui headers exist under third_party, add them to include path
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/raygui/src/raygui.h")
  target_include_directories(vray_demo PRIVATE ${CMAKE_SOURCE_DIR}/third_party/raygui/src)
elseif(EXISTS "${CMAKE_SOURCE_DIR}/third_party/raygui/raygui.h")
  target_include_directories(vray_demo PRIVATE ${CMAKE_SOURCE_DIR}/third_party/raygui)
endif()

if(DEFINED RAYLIB_TARGET)
  target_link_libraries(vray_demo PRIVATE ${RAYLIB_TARGET})
  target_link_libraries(mesh_tests PRIVATE ${RAYLIB_TARGET})
  target_link_libraries(smoke_tests PRIVATE ${RAYLIB_TARGET})
endif()

target_link_libraries(smoke_tests PRIVATE gtest_main)
add_test(NAME smoke_tests COMMAND smoke_tests)

# Helpful build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
