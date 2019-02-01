cmake_minimum_required(VERSION 3.1.0)
project(sprinkler)

set(WITHOUT_UNITTEST_BKUP ${without-unittest})
set(without-unittest YES)
add_subdirectory(histgram)
set(without-unittest ${WITHOUT_UNITTEST_BKUP})

set(CXX_VERSION 17)
include(lubee/cxx_options.cmake)
string(CONCAT CMAKE_CXX_FLAGS " -Wno-c99-extensions")

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
	Sql
	REQUIRED
)
find_package(Threads REQUIRED)

aux_source_directory(src SRC)
aux_source_directory(src/widget SRC)
aux_source_directory(src/sql SRC)
add_definitions(-DUNIX=${UNIX})
if(${UNIX})
	find_package(X11 REQUIRED)
	aux_source_directory(src/dep_linux SRC)
	include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}
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

# install時にシンボリックリンク作成(UNIX-only)
# macro(install_symlink filepath sympath)
    # install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${filepath} ${sympath})")
    # install(CODE "message(\"-- Created symlink: ${sympath} -> ${filepath}\")")
# endmacro(install_symlink)

macro(make_symlink filepath sympath)
	execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${filepath} ${sympath})
endmacro(make_symlink)
# Toml設定ファイルをランタイム出力先へコピー
make_symlink(${CMAKE_SOURCE_DIR}/settings.toml ${${PROJECT_NAME}_BINARY_DIR}/settings.toml)

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
	Qt5::Sql
	${CMAKE_THREAD_LIBS_INIT}
	${LINK_LIBS}
)
