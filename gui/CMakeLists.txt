add_custom_command(
	OUTPUT ${PREBUILT_MODULE_PATH}/gui-helpers.pcm
	COMMAND
		${MODULE_FLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/helpers.cppm
		-o ${PREBUILT_MODULE_PATH}/gui-helpers.pcm
	DEPENDS
		${PREBUILT_MODULE_PATH}/ftxui.pcm
		${PREBUILT_MODULE_PATH}/stl.pcm
		${PREBUILT_MODULE_PATH}/data.pcm
		${CMAKE_CURRENT_SOURCE_DIR}/helpers.cppm
)
add_custom_target(gui_helpers
	DEPENDS
		${PREBUILT_MODULE_PATH}/gui-helpers.pcm
)

###

add_custom_command(
	OUTPUT ${PREBUILT_MODULE_PATH}/gui-board.pcm
	COMMAND
		${MODULE_FLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/board.cppm
		-o ${PREBUILT_MODULE_PATH}/gui-board.pcm
	DEPENDS
		${PREBUILT_MODULE_PATH}/ftxui.pcm
		${PREBUILT_MODULE_PATH}/stl.pcm
		${PREBUILT_MODULE_PATH}/data.pcm
		${PREBUILT_MODULE_PATH}/gui-helpers.pcm
		${CMAKE_CURRENT_SOURCE_DIR}/board.cppm
)
add_custom_target(gui_board
	DEPENDS
		${PREBUILT_MODULE_PATH}/gui-board.pcm
)

###

add_custom_command(
	OUTPUT ${PREBUILT_MODULE_PATH}/gui-configuration.pcm
	COMMAND
		${MODULE_FLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/configuration.cppm
		-o ${PREBUILT_MODULE_PATH}/gui-configuration.pcm
	DEPENDS
		${PREBUILT_MODULE_PATH}/ftxui.pcm
		${PREBUILT_MODULE_PATH}/stl.pcm
		${PREBUILT_MODULE_PATH}/data.pcm
		${PREBUILT_MODULE_PATH}/gui-helpers.pcm
		${CMAKE_CURRENT_SOURCE_DIR}/configuration.cppm
)
add_custom_target(gui_configuration
	DEPENDS
		${PREBUILT_MODULE_PATH}/gui-configuration.pcm
)


###

add_custom_command(
	OUTPUT ${PREBUILT_MODULE_PATH}/gui.pcm
	COMMAND
		${MODULE_FLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/gui.cppm
		-o ${PREBUILT_MODULE_PATH}/gui.pcm
	DEPENDS
		${PREBUILT_MODULE_PATH}/ftxui.pcm
		${PREBUILT_MODULE_PATH}/stl.pcm
		${PREBUILT_MODULE_PATH}/data.pcm
		${PREBUILT_MODULE_PATH}/gui-helpers.pcm
		${PREBUILT_MODULE_PATH}/gui-board.pcm
		${PREBUILT_MODULE_PATH}/gui-configuration.pcm
		${CMAKE_CURRENT_SOURCE_DIR}/gui.cppm
)
add_custom_target(gui
	DEPENDS
		${PREBUILT_MODULE_PATH}/gui.pcm
)
add_dependencies(gui gui_board gui_configuration)
