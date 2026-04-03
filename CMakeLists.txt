cmake_minimum_required(VERSION 3.11)
project(levite C CXX)

include(FetchContent)

FetchContent_Declare(
  laskin
  GIT_REPOSITORY
    https://github.com/RauliL/laskin.git
  GIT_TAG
    v4.0.0
)
FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY
    https://github.com/d99kris/rapidcsv
  GIT_TAG
    v8.84
)
FetchContent_Declare(
  PeeloXdg
  GIT_REPOSITORY
    https://github.com/peelonet/peelo-xdg
  GIT_TAG
    v1.1.0
)
FetchContent_MakeAvailable(laskin rapidcsv PeeloXdg)

add_executable(
  levite
  ./src/cell.cpp
  ./src/color.cpp
  ./src/command.cpp
  ./src/coordinates.cpp
  ./src/event.cpp
  ./src/main.cpp
  ./src/range.cpp
  ./src/registry.cpp
  ./src/setting.cpp
  ./src/screen.cpp
  ./src/sheet.cpp
  ./src/termbox2.cpp
  ./src/utils.cpp
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
    PeeloXdg
)

install(
  TARGETS
    levite
  RUNTIME DESTINATION
    bin
)
