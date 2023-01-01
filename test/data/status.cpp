#include <expected>
#include <string>

#include <gtest/gtest.h>

#include <iostream>

import helpers;
import data;
import stl;

// This code is quite flacky and gets miscompiled every now and then
// not sure why, but then set is not called.

static bool is_blocked_one_dependency(task::tstatus status) {
  data::tstate *state =
      // std::make_unique<data::tstate>(
      new data::tstate{
          .tasks = {
              task{.id = 1, .title = "a", .status = status, .dependencies = {}},
              task{.id = 2,
                   .title = "b",
                   .status = task::tstatus::backlog,
                   .dependencies = {1}}}};

  data::set_state(state);
  return data::is_blocked(data::get_task(2));
}

TEST(status, is_blocked_one_dependency) {
  EXPECT_TRUE(is_blocked_one_dependency(task::tstatus::backlog));
  EXPECT_TRUE(is_blocked_one_dependency(task::tstatus::selected));
  EXPECT_TRUE(is_blocked_one_dependency(task::tstatus::progress));
  EXPECT_TRUE(is_blocked_one_dependency(task::tstatus::review));

  EXPECT_FALSE(is_blocked_one_dependency(task::tstatus::done));
  EXPECT_FALSE(is_blocked_one_dependency(task::tstatus::discarded));
}

static bool is_blocked_three_dependencies(std::array<task::tstatus, 3> status) {
  data::set_state(
      // std::make_unique<data::tstate>(
      new data::tstate{.tasks = {task{.id = 1,
                                      .title = "a",
                                      .status = status[0],
                                      .dependencies = {}},
                                 task{.id = 2,
                                      .title = "a",
                                      .status = status[1],
                                      .dependencies = {}},
                                 task{.id = 3,
                                      .title = "a",
                                      .status = status[2],
                                      .dependencies = {}},
                                 task{.id = 4,
                                      .title = "b",
                                      .status = task::tstatus::backlog,
                                      .dependencies = {1, 2, 3}}}});

  return data::is_blocked(data::get_task(4));
}

TEST(status, is_blocked_three_dependencies) {
  EXPECT_TRUE(is_blocked_three_dependencies({task::tstatus::backlog,
                                             task::tstatus::backlog,
                                             task::tstatus::backlog}));

  //

  EXPECT_TRUE(is_blocked_three_dependencies({task::tstatus::backlog,
                                             task::tstatus::backlog,
                                             task::tstatus::discarded}));

  EXPECT_TRUE(is_blocked_three_dependencies({task::tstatus::backlog,
                                             task::tstatus::discarded,
                                             task::tstatus::discarded}));

  EXPECT_FALSE(is_blocked_three_dependencies({task::tstatus::discarded,
                                              task::tstatus::discarded,
                                              task::tstatus::discarded}));
  //

  EXPECT_TRUE(is_blocked_three_dependencies(
      {task::tstatus::backlog, task::tstatus::backlog, task::tstatus::done}));

  EXPECT_TRUE(is_blocked_three_dependencies(
      {task::tstatus::backlog, task::tstatus::done, task::tstatus::done}));

  EXPECT_FALSE(is_blocked_three_dependencies(
      {task::tstatus::done, task::tstatus::done, task::tstatus::done}));
}

TEST(status, is_blocked_after) {
  // this tests needs a date provider as a setting
}

bool is_active_project(bool active) {
  data::set_state(
      // std::make_unique<data::tstate>(
      new data::tstate{
          .projects = {project{.id = 1, .name = "a", .active = active}},
          .tasks = {task{.id = 1, .project = 1, .title = "a"}}});

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
          .projects = {project{.id = 1, .name = "a", .active = project_active}},
          .groups = {group{
              .id = 1, .project = 1, .name = "a", .active = group_active}},
          .tasks = {task{.id = 1, .group = 1, .title = "a"}}});

  return data::is_active(data::get_task(1));
}

TEST(status, is_active_group) {
  EXPECT_TRUE(is_active_group(true, true));
  EXPECT_FALSE(is_active_group(true, false));
  EXPECT_FALSE(is_active_group(false, true));
  EXPECT_FALSE(is_active_group(false, false));
}
