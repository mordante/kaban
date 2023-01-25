cmake_minimum_required(VERSION 3.20.0 FATAL_ERROR)
project(kaban
	LANGUAGES CXX
)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
# Make sure all dependencies use the libc++.
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
include(ExternalProjects)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)
add_compile_options(-fexperimental-library)
add_link_options(-fexperimental-library)

set(PREBUILT_MODULE_PATH ${CMAKE_BINARY_DIR}/modules)
file(MAKE_DIRECTORY ${PREBUILT_MODULE_PATH})
add_compile_options(
	-fprebuilt-module-path=${PREBUILT_MODULE_PATH}
)
set(MODULE_FLAGS
	${CMAKE_CXX_COMPILER}
	${CMAKE_CXX_FLAGS}
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#	-O0 -g
	-std=c++2b
	-stdlib=libc++
	-fexperimental-library
	-fprebuilt-module-path=${PREBUILT_MODULE_PATH}
	--precompile
)

#add_library(state state.cpp)
#add_dependencies(state data stl)

add_custom_command(
	OUTPUT ${PREBUILT_MODULE_PATH}/stl.pcm
	COMMAND
		${MODULE_FLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/stl.cppm
		-o ${PREBUILT_MODULE_PATH}/stl.pcm
	DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/stl.cppm
)
add_custom_target(stl
	DEPENDS
		${PREBUILT_MODULE_PATH}/stl.pcm
)
add_custom_command(
	OUTPUT ${PREBUILT_MODULE_PATH}/data.pcm
	COMMAND
		${MODULE_FLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/data.cppm
		-o ${PREBUILT_MODULE_PATH}/data.pcm
	DEPENDS
		${PREBUILT_MODULE_PATH}/stl.pcm
		${CMAKE_CURRENT_SOURCE_DIR}/data.cppm
		#${CMAKE_CURRENT_SOURCE_DIR}/state.cpp
)
add_custom_target(data
	DEPENDS
		${PREBUILT_MODULE_PATH}/data.pcm
)

add_custom_command(
	OUTPUT ${PREBUILT_MODULE_PATH}/ftxui.pcm
	COMMAND
		${MODULE_FLAGS}
        -isystem ${ftxui_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/ftxui.cppm
		-o ${PREBUILT_MODULE_PATH}/ftxui.pcm
	DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/ftxui.cppm
)
add_custom_target(ftxui
	DEPENDS
		${PREBUILT_MODULE_PATH}/ftxui.pcm
)
add_executable(kaban
	kaban.cpp
)
add_dependencies(kaban data ftxui gui stl)

target_link_libraries(kaban
	PRIVATE
		${PREBUILT_MODULE_PATH}/data.pcm
		${PREBUILT_MODULE_PATH}/ftxui.pcm
		${PREBUILT_MODULE_PATH}/gui-board.pcm
		${PREBUILT_MODULE_PATH}/gui-configuration.pcm
		${PREBUILT_MODULE_PATH}/gui-helpers.pcm
		${PREBUILT_MODULE_PATH}/gui.pcm
		${PREBUILT_MODULE_PATH}/stl.pcm
		#state
		ftxui::screen
		ftxui::dom
		ftxui::component
)

add_subdirectory(gui)
add_subdirectory(scripts)
add_subdirectory(test)
