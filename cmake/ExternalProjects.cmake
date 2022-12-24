include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  add_subdirectory(
	  ${googletest_SOURCE_DIR}
	  ${googletest_BINARY_DIR}
	  EXCLUDE_FROM_ALL
  )
endif()

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  # v3.0.0 uses deprecated code, therefore use the latest master.
  GIT_TAG 4dc1a9fff94527a1acffb60f874d8800aecb7217
)
FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
