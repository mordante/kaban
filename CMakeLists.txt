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

set(COMPILER_DIAGNOSTICS
	-Werror
    -Weverything
    -Wno-exit-time-destructors # Don't care about exit time destructors.
    -Wno-padded # Don't care about the padding.
	-Wno-missing-prototypes # False postives with export.
	-Wno-global-constructors
	-Wno-weak-vtables
    # Ignore all compatibility with older C++ standards.
    -Wno-pre-c++20-compat
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-c++11-compat
    -Wno-c++14-compat
    -Wno-c++17-compat
    -Wno-c++20-compat
    # TODO Investigate whether the issues are real or false positives.
    -Wno-unsafe-buffer-usage
	# TODO in a different commit
	-Wno-deprecated-this-capture
	# TODO enable after module build is more stable
	-Wno-date-time
	-Wno-unused-command-line-argument
)
add_compile_options(${COMPILER_DIAGNOSTICS})


set(PREBUILT_MODULE_PATH ${CMAKE_BINARY_DIR}/modules)
file(MAKE_DIRECTORY ${PREBUILT_MODULE_PATH})
add_compile_options(
	-fprebuilt-module-path=${PREBUILT_MODULE_PATH}
)
set(MODULE_FLAGS
	${CMAKE_CXX_COMPILER}
	${CMAKE_CXX_FLAGS}
	${COMPILER_DIAGNOSTICS}
	-O0 -g
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
