cmake_minimum_required(VERSION 2.8)

project(cereal)

#set default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: Debug Release"
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(MY_FLAGS_DEBUG "-Wall -Wextra -Werror -g -pg -fno-inline")
set(MY_FLAGS_RELEASE "-Wall -Wextra -Werror  -O2")

set(CMAKE_C_FLAGS_DEBUG ${MY_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_DEBUG "${MY_FLAGS_DEBUG} -std=c++11")

set(CMAKE_C_FLAGS_RELEASE ${MY_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_RELEASE "${MY_FLAGS_RELEASE} -std=c++11")

message("Build type is ${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message("Release C   flags are ${CMAKE_C_FLAGS_RELEASE}")
  message("Release C++ flags are ${CMAKE_CXX_FLAGS_RELEASE}")
else()
  message("Debug   C   flags are ${CMAKE_C_FLAGS_DEBUG}")
  message("Debug   C++ flags are ${CMAKE_CXX_FLAGS_DEBUG}")
endif()


FILE(GLOB CEREAL_SRCS src/*.cpp)

add_library(cereal ${CEREAL_SRCS})
