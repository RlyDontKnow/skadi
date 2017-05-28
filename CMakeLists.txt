cmake_minimum_required(VERSION 3.8)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "" FORCE)
endif()

project(skadi CXX)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive-")
  # todo: enable more warnings; /Wall is too spammy due to missing silencing in the stdlib now
  add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)
find_package(Qt5 COMPONENTS
             Core
             Widgets
             Gui
             OpenGL
             REQUIRED)

add_definitions(-DPICOJSON_USE_INT64)

include_directories("include")
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB SRC "application/*.cpp" "source/*.cpp" "source/*.c" "include/detail/*.hpp")
file(GLOB INC "include/*.h" "include/*.hpp")

source_group("Source Files" FILES ${SRC})
source_group("Include Files" FILES ${INC})

add_executable(skadi ${SRC} ${INC})
target_link_libraries(skadi Qt5::Core Qt5::Widgets Qt5::Gui Qt5::OpenGL)
