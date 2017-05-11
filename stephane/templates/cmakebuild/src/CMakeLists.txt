cmake_minimum_required(VERSION 3.5)

project (gg)

set(CMAKE_VERBOSE_MAKEFILE true)

add_compile_options(-std=gnu++11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wunused-parameter")

if(${CMAKE_BUILD_TYPE} MATCHES "profile")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pg")
elseif(${CMAKE_BUILD_TYPE} MATCHES "coverage")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
endif()

include_directories(.)

file(GLOB SRC
  "*.cpp"
  "*/*.cpp"
  )

add_executable(${PROJECT_NAME} ${SRC})

set(EXECUTABLE_OUTPUT_PATH ../../src)

target_link_libraries(${PROJECT_NAME})
