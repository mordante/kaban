add_library(boost.ut)
target_sources(boost.ut
  PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES FILES
    ut.cppm
)
target_link_libraries(boost.ut
  PUBLIC
    ut
)

add_library(helpers)
target_sources(helpers
  PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES FILES
    helpers.cppm
)
target_link_libraries(helpers
  PUBLIC
    data
    boost.ut
)

add_executable(tests
  data/parse_basics.cpp
  data/parse_color.cpp
  data/parse_group.cpp
  data/parse_label.cpp
  data/parse_project.cpp
  data/parse_task.cpp
  data/status.cpp
  main.cpp
)

target_link_libraries(tests
  PRIVATE
    boost.ut
    helpers
    data
)
