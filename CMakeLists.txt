cmake_minimum_required(VERSION 2.8)

project(mqttcpp)


find_package(Boost COMPONENTS system program_options REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
endif()


#set default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: Debug Release"
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(MY_COMMON_FLAGS "-Wall -Wextra -Werror")
set(MY_FLAGS_DEBUG "${MY_COMMON_FLAGS} -g -fno-inline")
set(MY_FLAGS_RELEASE "${MY_COMMON_FLAGS} -O3 -flto")

set(CMAKE_CXX_FLAGS_DEBUG "${MY_FLAGS_DEBUG} -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "${MY_FLAGS_RELEASE} -std=c++14")

message("Build type is ${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message("Release C++ flags are ${CMAKE_CXX_FLAGS_RELEASE}")
else()
  message("Debug   C++ flags are ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

if(WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "-mwindows")
endif()

file(GLOB CEREAL_SRC cereal/src/*.cpp)
add_library(cereal STATIC ${CEREAL_SRC})


file(GLOB SRC src/*.cpp src/boost/*.cpp)
include_directories(src cereal/src src/boost gsl/include)
add_library(mqttcpp STATIC ${SRC})
target_link_libraries(mqttcpp cereal)

# add_executable(mqtt main.cpp)
# target_link_libraries(mqtt mqttcpp ${Boost_LIBRARIES})
# if(NOT WIN32)
#   target_link_libraries(mqtt pthread)
# endif()


if(CMAKE_BUILD_TYPE STREQUAL "Debug")

  message("Adding tests to the build")

  if(MQTT_ASAN)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
  endif()
  if(MQTT_PROFILE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg")
  endif()

  file(GLOB_RECURSE TEST_SRCS
    tests/*.cpp
    )

  add_executable(ut ${TEST_SRCS})
  target_link_libraries(ut mqttcpp)

endif()
