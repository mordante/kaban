cmake_minimum_required(VERSION 3.28.0 FATAL_ERROR)
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

# Based on libc++'s documentation
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fprebuilt-module-path=${CMAKE_BINARY_DIR}/_deps/std-build/CMakeFiles/std.dir/>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-nostdinc++>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-isystem>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${LIBCXX_BUILD}/include/c++/v1>)

add_link_options($<$<COMPILE_LANGUAGE:CXX>:-nostdlib++>)
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-L${LIBCXX_BUILD}/lib>)
add_link_options($<$<COMPILE_LANGUAGE:CXX>:-Wl,-rpath,${LIBCXX_BUILD}/lib>)
link_libraries(std c++)

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
	# TODO Investigate whether this should be fixed
	-Wno-switch-default
	# TODO enable after module build is more stable
	-Wno-read-modules-implicitly
	-Wno-date-time
	-Wno-unused-command-line-argument
	-Wno-ctad-maybe-unsupported # boost::ut
	-Wno-reserved-identifier # boost::ut
	-Wno-undef # boost::ut
    -Wno-deprecated # libc++
)
add_compile_options(${COMPILER_DIAGNOSTICS})

add_library(data)
target_sources(data
  PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES FILES
    data.cppm
)

add_library(ftxui)
target_sources(ftxui
  PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ftxui.cppm
)
target_include_directories(ftxui
	PRIVATE
		SYSTEM ${ftxui_SOURCE_DIR}/include
)

add_executable(kaban
	kaban.cpp
)
target_link_libraries(kaban PRIVATE data gui ftxui)

target_link_libraries(kaban
	PRIVATE
		ftxui::screen
		ftxui::dom
		ftxui::component
		ftxui
		data
		gui
)

add_subdirectory(gui)
add_subdirectory(scripts)
add_subdirectory(test)
