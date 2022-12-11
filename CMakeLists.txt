cmake_minimum_required(VERSION 3.20.0 FATAL_ERROR)
project(kaban
	LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)
add_compile_options(-stdlib=libc++ -fexperimental-library)
add_executable(kaban
	kaban.cpp
)
target_link_libraries(kaban
	PRIVATE
		c++experimental
		c++
)
