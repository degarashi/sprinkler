cmake_minimum_required(VERSION 3.1.0)
project(sprinkler)

set(without-unittest YES)
add_subdirectory(src/histgram)

set(CXX_VERSION 17)
include(src/lubee/cxx_options.cmake)

if(${CMAKE_CXX_COMPILER_ID} MATCHES ".*(C|c)lang")
	message(STATUS "Using clang...")
	string(CONCAT CMAKE_CXX_FLAGS " -Wno-gnu-anonymous-struct")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	message(STATUS "Using gcc...")
	string(CONCAT CMAKE_CXX_FLAGS " -Wno-pedantic")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS
	Widgets
	LinguistTools
	REQUIRED
)
find_package(Threads REQUIRED)

aux_source_directory(src SRC)
if(${UNIX})
	find_package(X11 REQUIRED)
	aux_source_directory(src/dep_linux SRC)
	include_directories(
		src/dep_linux
		${X11_INCLUDE_DIR}
	)
	set(LINK_LIBS ${LINK_LIBS} ${X11_LIBRARIES})
elseif(${WIN32})
	aux_source_directory(src/dep_win SRC)
	set(SRC ${SRC}
		src/dep_win/ghook/wevent.cpp
		src/dep_win/ghook/wmutex.cpp
	)
	include_directories(
		src/dep_win
	)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-subsystem,windows")
else()
	message(FATAL_ERROR "Unknown target system(neither unix nor win32)")
endif()

set(TRANSLATION_DIR translation)
file(GLOB_RECURSE UI_FILES src/*.ui)
set(TS_FILES
	${TRANSLATION_DIR}/sprinkler_ja.ts
)
set(TRANSLATION_TARGET_FILES
	${SRC}
	${UI_FILES}
)
qt5_create_translation(QM_FILES
	${TRANSLATION_TARGET_FILES}
	${TS_FILES}
)

add_executable(
	${PROJECT_NAME}
	${SRC}
	${UI_FILES}
	${QM_FILES}
)
target_link_libraries(
	${PROJECT_NAME}
	histgram
	Qt5::Widgets
	${CMAKE_THREAD_LIBS_INIT}
	${LINK_LIBS}
)