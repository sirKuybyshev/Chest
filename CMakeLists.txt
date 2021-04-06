cmake_minimum_required(VERSION 3.19)
project(Semestr5)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -g")

add_executable(Chest Chest/main.cpp)
add_executable(TypeQueue TypeQueue/main.cpp TypeQueue/utils.h TypeQueue/tQueue.h)
add_executable(Terms Terms/main.cpp Terms/vault.h)