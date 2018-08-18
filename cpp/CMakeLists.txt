project(NebulasDataPlatform)
cmake_minimum_required(VERSION 3.0)
message(STATUS "This is BINARY dir" ${PROJECT_BINARY_DIR})
message(STATUS "This is SOURCE dir" ${PROJECT_SOURCE_DIR})

message(STATUS "boost dir " $ENV{BOOST_DIR})
message(STATUS "db url " $ENV{DB_URL})
add_definitions(-std=c++11)

if(Release)
  message(STATUS "This is a Release version!")
  add_definitions(-O2)
  add_definitions(-DNDEBUG)
else(Release)
  message(STATUS "This is a Debug version!")
  add_definitions(-g)
  add_definitions(-O0)
endif(Release)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_definitions(-DDB_URL=$ENV{DB_URL})
add_definitions(-DDB_USER_NAME=$ENV{DB_USER_NAME})
add_definitions(-DDB_PASSWORD=$ENV{DB_PASSWORD})
add_definitions(-DNEBULAS_DB=$ENV{NEBULAS_DB})


include_directories(${PROJECT_SOURCE_DIR}/src/)
include_directories(/usr/local/include)
include_directories($ENV{BOOST_DIR}/include)
include_directories($ENV{BOOST_DIR})

link_directories($ENV{BOOST_DIR}/lib)
link_directories(${PROJECT_BINARY_DIR})
link_directories(/usr/local/lib)

add_subdirectory(src)
enable_testing()

set(common_libs tga_engine fuerte velocypack boost_system pthread curl ssl crypto boost_date_time glog)

