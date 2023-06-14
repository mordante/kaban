import ut_helpers;

import data;

import boost.ut;

import std;

namespace {

using namespace boost::ut::literals;

bool is_blocked_one_dependency(data::ttask::tstatus status) {
  std::expected<void, std::nullptr_t> result =
      data::set_state(std::make_unique<data::tstate>(data::tstate{
          .tasks = {data::ttask{.id = 100,
                                .title = "a",
                                .status = status,
                                .dependencies = {}},
                    data::ttask{.id = 200,
                                .title = "b",
                                .status = data::ttask::tstatus::backlog,
                                .dependencies = {100}}}}));

  expect_true(result);

  return data::is_blocked(data::get_task(200));
}

bool is_blocked_one_requirement_one_task(data::ttask::tstatus status) {
  std::expected<void, std::nullptr_t> result =
      data::set_state(std::make_unique<data::tstate>(data::tstate{
          .projects = {data::tproject{.id = 1, .name = "a"}},
          .groups = {data::tgroup{.id = 10, .project = 1, .name = "a"}},
          .tasks = {data::ttask{
                        .id = 100,
                        .group = 10,
                        .title = "a",
                        .status = status,
                    },
                    data::ttask{.id = 200,
                                .title = "b",
                                .status = data::ttask::tstatus::backlog,
                                .requirements = {10}}}}));

  expect_true(result);

  return data::is_blocked(data::get_task(200));
}

bool is_blocked_two_requirements_with_three_tasks(
    std::array<data::ttask::tstatus, 3> status) {
  std::expected<void, std::nullptr_t> result =
      data::set_state(std::make_unique<data::tstate>(data::tstate{
          .projects = {data::tproject{.id = 1, .name = "a"}},
          .groups = {data::tgroup{.id = 10, .project = 1, .name = "a"},
                     data::tgroup{.id = 20, .project = 1, .name = "b"}},
          .tasks = {data::ttask{.id = 100,
                                .group = 10,
                                .title = "a",
                                .status = status[0],
                                .dependencies = {}},
                    data::ttask{.id = 200,
                                .group = 20,
                                .title = "a",
                                .status = status[1],
                                .dependencies = {}},
                    data::ttask{.id = 300,
                                .group = 20,
                                .title = "a",
                                .status = status[2],
                                .dependencies = {}},
                    data::ttask{.id = 400,
                                .title = "b",
                                .status = data::ttask::tstatus::backlog,
                                .requirements = {10, 20}}}}));

  expect_true(result);

  return data::is_blocked(data::get_task(400));
}
bool is_blocked_three_dependencies(std::array<data::ttask::tstatus, 3> status) {
  std::expected<void, std::nullptr_t> result =
      data::set_state(std::make_unique<data::tstate>(data::tstate{
          .tasks = {data::ttask{.id = 100,
                                .title = "a",
                                .status = status[0],
                                .dependencies = {}},
                    data::ttask{.id = 200,
                                .title = "a",
                                .status = status[1],
                                .dependencies = {}},
                    data::ttask{.id = 300,
                                .title = "a",
                                .status = status[2],
                                .dependencies = {}},
                    data::ttask{.id = 400,
                                .title = "b",
                                .status = data::ttask::tstatus::backlog,
                                .dependencies = {100, 200, 300}}}}));

  expect_true(result);

  return data::is_blocked(data::get_task(400));
};

bool is_active_project(bool active) {
  std::expected<void, std::nullptr_t> result =
      data::set_state(std::make_unique<data::tstate>(data::tstate{
          .projects = {data::tproject{.id = 1, .name = "a", .active = active}},
          .tasks = {data::ttask{.id = 100, .project = 1, .title = "a"}}}));

  expect_true(result);

  return data::is_active(data::get_task(100));
}

bool is_active_group(bool project_active, bool group_active) {
  std::expected<void, std::nullptr_t> result =
      data::set_state(std::make_unique<data::tstate>(data::tstate{
          .projects = {data::tproject{
              .id = 1, .name = "a", .active = project_active}},
          .groups = {data::tgroup{
              .id = 10, .project = 1, .name = "a", .active = group_active}},
          .tasks = {data::ttask{.id = 100, .group = 10, .title = "a"}}}));

  expect_true(result);

  return data::is_active(data::get_task(100));
}

boost::ut::suite<"status"> suite = [] {
  "is_blocked_one_dependency"_test = [] {
    expect_true(is_blocked_one_dependency(data::ttask::tstatus::backlog));
    expect_true(is_blocked_one_dependency(data::ttask::tstatus::selected));
    expect_true(is_blocked_one_dependency(data::ttask::tstatus::progress));
    expect_true(is_blocked_one_dependency(data::ttask::tstatus::review));

    expect_false(is_blocked_one_dependency(data::ttask::tstatus::done));
    expect_false(is_blocked_one_dependency(data::ttask::tstatus::discarded));
  };

  "is_blocked_three_dependencies"_test = [] {
    expect_true(is_blocked_three_dependencies({data::ttask::tstatus::backlog,
                                               data::ttask::tstatus::backlog,
                                               data::ttask::tstatus::backlog}));

    //

    expect_true(is_blocked_three_dependencies(
        {data::ttask::tstatus::backlog, data::ttask::tstatus::backlog,
         data::ttask::tstatus::discarded}));

    expect_true(is_blocked_three_dependencies(
        {data::ttask::tstatus::backlog, data::ttask::tstatus::discarded,
         data::ttask::tstatus::discarded}));

    expect_false(is_blocked_three_dependencies(
        {data::ttask::tstatus::discarded, data::ttask::tstatus::discarded,
         data::ttask::tstatus::discarded}));
    //

    expect_true(is_blocked_three_dependencies({data::ttask::tstatus::backlog,
                                               data::ttask::tstatus::backlog,
                                               data::ttask::tstatus::done}));

    expect_true(is_blocked_three_dependencies({data::ttask::tstatus::backlog,
                                               data::ttask::tstatus::done,
                                               data::ttask::tstatus::done}));

    expect_false(is_blocked_three_dependencies({data::ttask::tstatus::done,
                                                data::ttask::tstatus::done,
                                                data::ttask::tstatus::done}));
  };

  "is_blocked_one_requirement_one_task"_test = [] {
    expect_true(
        is_blocked_one_requirement_one_task(data::ttask::tstatus::backlog));
    expect_true(
        is_blocked_one_requirement_one_task(data::ttask::tstatus::selected));
    expect_true(
        is_blocked_one_requirement_one_task(data::ttask::tstatus::progress));
    expect_true(
        is_blocked_one_requirement_one_task(data::ttask::tstatus::review));

    expect_false(
        is_blocked_one_requirement_one_task(data::ttask::tstatus::done));
    expect_false(
        is_blocked_one_requirement_one_task(data::ttask::tstatus::discarded));
  };

  "is_blocked_two_requirements_with_three_tasks"_test = [] {
    expect_true(is_blocked_two_requirements_with_three_tasks(
        {data::ttask::tstatus::backlog, data::ttask::tstatus::backlog,
         data::ttask::tstatus::backlog}));

    //

    expect_true(is_blocked_two_requirements_with_three_tasks(
        {data::ttask::tstatus::backlog, data::ttask::tstatus::backlog,
         data::ttask::tstatus::discarded}));

    expect_true(is_blocked_two_requirements_with_three_tasks(
        {data::ttask::tstatus::backlog, data::ttask::tstatus::discarded,
         data::ttask::tstatus::discarded}));

    expect_false(is_blocked_two_requirements_with_three_tasks(
        {data::ttask::tstatus::discarded, data::ttask::tstatus::discarded,
         data::ttask::tstatus::discarded}));
    //

    expect_true(is_blocked_two_requirements_with_three_tasks(
        {data::ttask::tstatus::backlog, data::ttask::tstatus::backlog,
         data::ttask::tstatus::done}));

    expect_true(is_blocked_two_requirements_with_three_tasks(
        {data::ttask::tstatus::backlog, data::ttask::tstatus::done,
         data::ttask::tstatus::done}));

    expect_false(is_blocked_two_requirements_with_three_tasks(
        {data::ttask::tstatus::done, data::ttask::tstatus::done,
         data::ttask::tstatus::done}));
  };

  "is_blocked_after"_test = [] {
    // this tests needs a date provider as a setting
  };

  "is_active_project"_test = [] {
    expect_true(is_active_project(true));
    expect_false(is_active_project(false));
  };

  "is_active_group"_test = [] {
    expect_true(is_active_group(true, true));
    expect_false(is_active_group(true, false));
    expect_false(is_active_group(false, true));
    expect_false(is_active_group(false, false));
  };
};

} // namespace
