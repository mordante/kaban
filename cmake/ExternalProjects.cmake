include(FetchContent)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
)
FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


set(BOOST_UT_ALLOW_CPM_USE OFF)
FetchContent_Declare(
	ut
    GIT_REPOSITORY https://github.com/mordante/ut.git
	# GIT_REPOSITORY https://github.com/boost-ext/ut
	GIT_TAG modular_non_header_build
	GIT_SHALLOW    TRUE
)
FetchContent_GetProperties(ut)
if(NOT boost_ut_POPULATED)
  FetchContent_Populate(ut)
  add_subdirectory(${ut_SOURCE_DIR} ${ut_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
  std
  URL "file://${LIBCXX_BUILD}/modules/c++/v1/"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_GetProperties(std)
if(NOT std_POPULATED)
  set(CMAKE_CXX_STANDARD 23)
  FetchContent_Populate(std)
  add_subdirectory(${std_SOURCE_DIR} ${std_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
