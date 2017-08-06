cmake_minimum_required(VERSION 3.7)
project(tcp_win)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp main.h mdspi.cpp mdspi.h strategy.cpp strategy.h StructFunction.cpp StructFunction.h traderspi.cpp traderspi.h RdWrini.h RdWrini.cpp)
add_executable(tcp_win ${SOURCE_FILES})