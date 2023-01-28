
# Due to the external library include this can't use the function.
# TODO solve this by allowing additional commands or switch to newer
# CMake once available.
add_custom_command(
    OUTPUT ${PREBUILT_MODULE_PATH}/helpers.pcm
	COMMAND
		${CMAKE_CXX_COMPILER}
		${CMAKE_CXX_FLAGS}
		${MODULE_FLAGS}
		-isystem ${googletest_SOURCE_DIR}/googletest/include
		${CMAKE_CURRENT_SOURCE_DIR}/helpers.cppm
		-o ${PREBUILT_MODULE_PATH}/helpers.pcm
	DEPENDS
	  	${PREBUILT_MODULE_PATH}/data.pcm
		${CMAKE_CURRENT_SOURCE_DIR}/helpers.cppm
)
add_custom_target(helpers
	DEPENDS 
	  ${PREBUILT_MODULE_PATH}/helpers.pcm
)

add_executable(tests
	data/parse_basics.cpp
	data/parse_color.cpp
	data/parse_group.cpp
	data/parse_label.cpp
	data/parse_project.cpp
	data/parse_task.cpp
	data/status.cpp
)
add_dependencies(tests
	helpers
	data
	#state
	stl
)
target_link_libraries(tests
	PRIVATE
		gtest_main
		${PREBUILT_MODULE_PATH}/data.pcm
		${PREBUILT_MODULE_PATH}/helpers.pcm
		${PREBUILT_MODULE_PATH}/stl.pcm
		#state
)
