cmake_minimum_required(VERSION 3.10)
project(StackTest)

enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(GTEST_INCLUDE_DIRS "/usr/local/Cellar/googletest/1.15.0/include")
set(GTEST_LIBRARIES "/usr/local/Cellar/googletest/1.15.0/lib/libgtest.a;/usr/local/Cellar/googletest/1.15.0/lib/libgtest_main.a")

include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/../include)

add_executable(StackTest stack_test.cc)

target_link_libraries(StackTest ${GTEST_LIBRARIES} pthread StackLib)

add_test(NAME StackTest COMMAND StackTest)
