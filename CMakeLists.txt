cmake_minimum_required(VERSION 2.8.8)

include_directories(include)
include_directories(/usr/include/libxml2)
file(GLOB LIB_SOURCES "src/*.c")
add_definitions(-std=c11)

project(cmeta)
add_library(${CMAKE_PROJECT_NAME} STATIC ${LIB_SOURCES})

install(
    DIRECTORY include/ DESTINATION /usr/local/include
    FILES_MATCHING PATTERN "*.h"
)
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION /usr/local/lib)

project(main)
add_executable(${CMAKE_PROJECT_NAME} "main.c" ${LIB_SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME} xml2)
