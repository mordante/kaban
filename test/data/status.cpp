#include <expected>
#include <string>

#include <gtest/gtest.h>

#include <iostream>

import helpers;
import data;
import stl;

// This code is quite flacky and gets miscompiled every now and then
// not sure why, but then set is not called.

static bool is_blocked_one_dependency(data::ttask::tstatus status) {
  data::tstate *state =
      // std::make_unique<data::tstate>(
      new data::tstate{
          .tasks = {
              data::ttask{
                  .id = 1, .title = "a", .status = status, .dependencies = {}},
              data::ttask{.id = 2,
                          .title = "b",
                          .status = data::ttask::tstatus::backlog,
                          .dependencies = {1}}}};

  data::set_state(state);
  return data::is_blocked(data::get_task(2));
}

TEST(status, is_blocked_one_dependency) {
  EXPECT_TRUE(is_blocked_one_dependency(data::ttask::tstatus::backlog));
  EXPECT_TRUE(is_blocked_one_dependency(data::ttask::tstatus::selected));
  EXPECT_TRUE(is_blocked_one_dependency(data::ttask::tstatus::progress));
  EXPECT_TRUE(is_blocked_one_dependency(data::ttask::tstatus::review));

  EXPECT_FALSE(is_blocked_one_dependency(data::ttask::tstatus::done));
  EXPECT_FALSE(is_blocked_one_dependency(data::ttask::tstatus::discarded));
}

static bool
is_blocked_three_dependencies(std::array<data::ttask::tstatus, 3> status) {
  data::set_state(
      // std::make_unique<data::tstate>(
      new data::tstate{
          .tasks = {data::ttask{.id = 1,
                                .title = "a",
                                .status = status[0],
                                .dependencies = {}},
                    data::ttask{.id = 2,
                                .title = "a",
                                .status = status[1],
                                .dependencies = {}},
                    data::ttask{.id = 3,
                                .title = "a",
                                .status = status[2],
                                .dependencies = {}},
                    data::ttask{.id = 4,
                                .title = "b",
                                .status = data::ttask::tstatus::backlog,
                                .dependencies = {1, 2, 3}}}});

  return data::is_blocked(data::get_task(4));
}

TEST(status, is_blocked_three_dependencies) {
  EXPECT_TRUE(is_blocked_three_dependencies({data::ttask::tstatus::backlog,
                                             data::ttask::tstatus::backlog,
                                             data::ttask::tstatus::backlog}));

  //

  EXPECT_TRUE(is_blocked_three_dependencies({data::ttask::tstatus::backlog,
                                             data::ttask::tstatus::backlog,
                                             data::ttask::tstatus::discarded}));

  EXPECT_TRUE(is_blocked_three_dependencies({data::ttask::tstatus::backlog,
                                             data::ttask::tstatus::discarded,
                                             data::ttask::tstatus::discarded}));

  EXPECT_FALSE(is_blocked_three_dependencies(
      {data::ttask::tstatus::discarded, data::ttask::tstatus::discarded,
       data::ttask::tstatus::discarded}));
  //

  EXPECT_TRUE(is_blocked_three_dependencies({data::ttask::tstatus::backlog,
                                             data::ttask::tstatus::backlog,
                                             data::ttask::tstatus::done}));

  EXPECT_TRUE(is_blocked_three_dependencies({data::ttask::tstatus::backlog,
                                             data::ttask::tstatus::done,
                                             data::ttask::tstatus::done}));

  EXPECT_FALSE(is_blocked_three_dependencies({data::ttask::tstatus::done,
                                              data::ttask::tstatus::done,
                                              data::ttask::tstatus::done}));
}

TEST(status, is_blocked_after) {
  // this tests needs a date provider as a setting
}

bool is_active_project(bool active) {
  data::set_state(
      // std::make_unique<data::tstate>(
      new data::tstate{
          .projects = {data::tproject{.id = 1, .name = "a", .active = active}},
          .tasks = {data::ttask{.id = 1, .project = 1, .title = "a"}}});

  return data::is_active(data::get_task(1));
}

TEST(status, is_active_project) {
  EXPECT_TRUE(is_active_project(true));
  EXPECT_FALSE(is_active_project(false));
}

bool is_active_group(bool project_active, bool group_active) {
  data::set_state(
      // std::make_unique<data::tstate>(
      new data::tstate{
          .projects = {data::tproject{
              .id = 1, .name = "a", .active = project_active}},
          .groups = {data::tgroup{
              .id = 1, .project = 1, .name = "a", .active = group_active}},
          .tasks = {data::ttask{.id = 1, .group = 1, .title = "a"}}});

  return data::is_active(data::get_task(1));
}

TEST(status, is_active_group) {
  EXPECT_TRUE(is_active_group(true, true));
  EXPECT_FALSE(is_active_group(true, false));
  EXPECT_FALSE(is_active_group(false, true));
  EXPECT_FALSE(is_active_group(false, false));
}
