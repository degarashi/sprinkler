cmake_minimum_required(VERSION 3.1.0)
project("ghook${BITVERSION}" VERSION 0.0.1)

set(without-unittest YES)

set(CXX_VERSION 17)
include(cxx_options.cmake)

if(${CMAKE_CXX_COMPILER_ID} MATCHES ".*(C|c)lang")
	message(STATUS "Using clang...")
	string(CONCAT CMAKE_CXX_FLAGS " -Wno-gnu-anonymous-struct")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	message(STATUS "Using gcc...")
	string(CONCAT CMAKE_CXX_FLAGS " -Wno-pedantic")
endif()

add_definitions(-DBITVERSION=${BITVERSION})
add_definitions(-DBUILD_DLL)
unset(SRC)
aux_source_directory(. SRC)
if(NOT DEFINED WIN32)
	message(FATAL_ERROR "This project is Windows only")
endif()

add_library(${PROJECT_NAME} SHARED ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES
						VERSION ${PROJECT_VERSION}
						SOVERSION 0)
# target_link_libraries(${PROJECT_NAME})
