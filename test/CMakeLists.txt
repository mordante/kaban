add_library(helpers)
target_sources(helpers
  PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES FILES
    helpers.cppm
)
target_include_directories(helpers
	PRIVATE
		SYSTEM ${googletest_SOURCE_DIR}/googletest/include
)
target_link_libraries(helpers PUBLIC data)

add_executable(tests
	data/parse_basics.cpp
	data/parse_color.cpp
	data/parse_group.cpp
	data/parse_label.cpp
	data/parse_project.cpp
	data/parse_task.cpp
	data/status.cpp
)

target_link_libraries(tests
	PRIVATE
		gtest_main
		helpers
)
