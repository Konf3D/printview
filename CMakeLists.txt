cmake_minimum_required(VERSION 3.25)
project(MyApp)

find_library(NETSNMPMIBS "netsnmpmibs")
find_library(NETSNMP "netsnmp")
find_library(NETSNMPAGENT "netsnmpagent")
find_package(Xlnt CONFIG REQUIRED)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE xlnt::xlnt ${NETSNMPAGENT} ${NETSNMPMIBS} ${NETSNMP})