cmake_minimum_required(VERSION 3.11)
project(levite C CXX)

include(FetchContent)

FetchContent_Declare(
  laskin
  GIT_REPOSITORY
    https://github.com/RauliL/laskin.git
  GIT_TAG
    v2.2.0
)
FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY
    https://github.com/d99kris/rapidcsv
  GIT_TAG
    v8.84
)
FetchContent_MakeAvailable(laskin rapidcsv)

add_executable(
  levite
  ./src/event.cpp
  ./src/main.cpp
  ./src/screen.cpp
  ./src/sheet.cpp
  ./src/termbox2.cpp
)

target_compile_options(
  levite
  PRIVATE
    -Wall -Werror
)

target_compile_features(
  levite
  PRIVATE
    cxx_std_17
)

target_link_libraries(
  levite
  PRIVATE
    laskin
    rapidcsv
)

install(
  TARGETS
    levite
  RUNTIME DESTINATION
    bin
)
