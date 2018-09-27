cmake_minimum_required(VERSION 3.1.0)

if(NOT DEFINED BITVERSION)
	message(FATAL_ERROR "variable BITVERSION not defined")
endif()
project("watcher${BITVERSION}" VERSION 0.0.1)
add_subdirectory(ghook)

set(CXX_VERSION 17)
include(cxx_options.cmake)

if(NOT DEFINED WIN32)
	message(FATAL_ERROR "This project is Windows only")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-subsystem,windows")
add_definitions(-DBITVERSION=${BITVERSION})
unset(SRC)
aux_source_directory(. SRC)
add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME}
	"ghook${BITVERSION}"
)
