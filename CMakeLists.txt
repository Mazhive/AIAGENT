cmake_minimum_required(VERSION 3.16)

project(AdvancedSnake
  VERSION 1.0.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(advanced_snake
  src/main.cpp
  src/Game.cpp
  src/Snake.cpp
  src/Food.cpp
  src/Hud.cpp
  src/Random.cpp
  src/Button.cpp
  src/HighScoreStore.cpp
)

target_include_directories(advanced_snake PRIVATE include)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
target_link_libraries(advanced_snake PRIVATE sfml-graphics sfml-window sfml-system)

if (MSVC)
  target_compile_options(advanced_snake PRIVATE /W4)
else()
  target_compile_options(advanced_snake PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(TARGET advanced_snake POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:advanced_snake>/assets"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets"
          "$<TARGET_FILE_DIR:advanced_snake>/assets"
  COMMENT "Copying assets to output directory"
)

