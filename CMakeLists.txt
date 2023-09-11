cmake_minimum_required(VERSION 3.2)

# set(CMAKE_CXX_COMPILER "clang++")

project(xxx)
ADD_DEFINITIONS(-std=c++17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

# directories
include_directories("./")
include_directories("deps")
include_directories("xcomm")


## tests
include(benchs/Rolex/tests/tests.cmake)
enable_testing()

add_test(NAME test COMMAND coretest)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose
                  DEPENDS coretest )


set(LOG_SRC "./deps/r2/src/logging.cc"  "./deps/r2/src/sshed.cc"  "./benchs/terminate.cc")



# rolex
# file(GLOB rolex_SORUCES ""  "./benchs/Rolex/rolex.cc"  "./deps/r2/src/logging.cc"  "./deps/r2/src/sshed.cc"  "./benchs/terminate.cc" )
# add_executable(rolex ${rolex_SORUCES} )
# target_link_libraries(rolex gflags ibverbs pthread boost_system boost_coroutine)

file(GLOB rolex_server_SORUCES ""  "./benchs/Rolex/rolex_server.cc"  "./deps/r2/src/logging.cc" )
add_executable(rolex_server ${rolex_server_SORUCES} )
target_link_libraries(rolex_server boost_system boost_coroutine pthread gflags ibverbs )
file(GLOB rolex_client_SORUCES ""  "./benchs/Rolex/rolex_client.cc"  "./deps/r2/src/logging.cc"  "./deps/r2/src/sshed.cc" )
add_executable(rolex_client ${rolex_client_SORUCES} )
target_link_libraries(rolex_client boost_system boost_coroutine pthread gflags ibverbs )