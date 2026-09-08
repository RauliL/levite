cmake_minimum_required(VERSION 3.32)
project(levite C CXX)

include(FetchContent)

FetchContent_Declare(
  laskin
  GIT_REPOSITORY
    https://github.com/RauliL/laskin.git
  GIT_TAG
    v8.1.0
)
FetchContent_Declare(
  rapidcsv
  GIT_REPOSITORY
    https://github.com/d99kris/rapidcsv
  GIT_TAG
    v9.07
)
FetchContent_Declare(
  PeeloXdg
  GIT_REPOSITORY
    https://github.com/peelonet/peelo-xdg
  GIT_TAG
    v1.2.1
)
FetchContent_Declare(
  libterm
  GIT_REPOSITORY
    https://github.com/rizukirr/libterm
  GIT_TAG
    v0.1.2
)

# We do not want to build Laskin CLI, GUI or any other fluff than the
# interpreter library.
set(LASKIN_ENABLE_CLI OFF CACHE BOOL "" FORCE)
set(LASKIN_ENABLE_GUI OFF CACHE BOOL "" FORCE)
set(LASKIN_ENABLE_2CPP OFF CACHE BOOL "" FORCE)
set(LASKIN_ENABLE_WEB OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(laskin rapidcsv PeeloXdg libterm)

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
    Libterm::libterm
)

install(
  TARGETS
    levite
  RUNTIME DESTINATION
    bin
)
