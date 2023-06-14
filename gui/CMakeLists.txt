add_library(gui)
target_sources(gui
  PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES FILES
    helpers.cppm
	board.cppm
	configuration.cppm
	gui.cppm
)

target_link_libraries(gui PUBLIC data ftxui)
