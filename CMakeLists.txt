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

add_executable(tests
  tests/smoke_tests.cpp
  tests/boss_play_tests.cpp
  tests/mech_tests.cpp
  tests/world_tests.cpp
  tests/entity_grid_tests.cpp
  tests/card_game_tests.cpp
  tests/config_tests.cpp
  tests/raii_and_backend_tests.cpp
  src/boss/boss.cpp
  src/boss/bossState.h
  src/boss/bossStartupState.cpp
  src/boss/bossCardSelectState.cpp
  src/boss/bossNpcSelectState.cpp
  src/boss/bossPlayState.cpp
  src/boss/bossEndGameState.cpp
  src/config.cpp
  src/platform/platform.cpp
  src/platform/raylib_window.cpp
  src/platform/raylib_input.cpp
  src/platform/raylib_renderer.cpp
  src/utils/meshMech.cpp
  src/utils/meshGenerateUtils.cpp
  src/utils/meshMathUtils.cpp
  src/utils/meshProcessUtils.cpp
  src/utils/meshExtraShapeUtils.cpp
  src/utils/meshCompositeUtils.cpp
  src/utils/luaUtils.cpp
  src/rlights_impl.cpp
  src/world/world.cpp
  src/grid.cpp
  src/card.cpp
  src/game.cpp
)

# Shared include path and defines
target_include_directories(tests PRIVATE src)
target_compile_definitions(tests PRIVATE _CRT_SECURE_NO_WARNINGS)

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
  target_link_libraries(tests PRIVATE ${RAYLIB_TARGET})
endif()

target_link_libraries(tests PRIVATE gtest_main)
add_test(NAME tests COMMAND tests)

# Helpful build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
