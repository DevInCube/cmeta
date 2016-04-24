cmake_minimum_required(VERSION 2.8.8)
project(cmeta)

set(CMETA_ROOT_PATH .)
set(CMETA_SOURCES_PATH ${CMETA_ROOT_PATH}/src)
set(CMETA_INCLUDE_PATH ${CMETA_ROOT_PATH}/include)

file(GLOB_RECURSE CMETA_SOURCES
	${CMETA_SOURCES_PATH}/*.h
	${CMETA_SOURCES_PATH}/*.c
)

file(GLOB_RECURSE CMETA_HEADERS
	${CMETA_INCLUDE_PATH}/*.h
)

include_directories(${CMETA_INCLUDE_PATH})
add_definitions(-std=c99)
add_library(cmeta STATIC ${CMETA_SOURCES} ${CMETA_HEADERS})
