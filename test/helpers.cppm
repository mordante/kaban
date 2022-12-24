module;
#include <chrono>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

export module helpers;

import data;

export std::string format(const data::tparse_error &error) {
  return std::format(R"(Parsing error.
line    {}: {}
message {}
)",
                     error.line_no, error.line, error.message);
}

export void expect_eq(const data::tparse_error &lhs,
                      const data::tparse_error &rhs) {

  EXPECT_EQ(lhs.line_no, rhs.line_no);
  EXPECT_EQ(lhs.line, rhs.line);
  EXPECT_EQ(lhs.message, rhs.message);
}

export void expect_eq(const label &lhs, const label &rhs) {
  EXPECT_EQ(lhs.id, rhs.id);
  EXPECT_EQ(lhs.name, rhs.name);
  EXPECT_EQ(lhs.description, rhs.description);
  EXPECT_EQ(lhs.color, rhs.color);
}

export void expect_eq(const project &lhs, const project &rhs) {
  EXPECT_EQ(lhs.id, rhs.id);
  EXPECT_EQ(lhs.name, rhs.name);
  EXPECT_EQ(lhs.description, rhs.description);
  EXPECT_EQ(lhs.color, rhs.color);
  EXPECT_EQ(lhs.active, rhs.active);
}

export void expect_eq(const group &lhs, const group &rhs) {
  EXPECT_EQ(lhs.id, rhs.id);
  EXPECT_EQ(lhs.project, rhs.project);
  EXPECT_EQ(lhs.name, rhs.name);
  EXPECT_EQ(lhs.description, rhs.description);
  EXPECT_EQ(lhs.color, rhs.color);
  EXPECT_EQ(lhs.active, rhs.active);
}

export void expect_eq(const task &lhs, const task &rhs) {
  EXPECT_EQ(lhs.id, rhs.id);
  EXPECT_EQ(lhs.project, rhs.project);
  EXPECT_EQ(lhs.group, rhs.group);
  EXPECT_EQ(lhs.title, rhs.title);
  EXPECT_EQ(lhs.description, rhs.description);
  EXPECT_EQ(lhs.status, rhs.status);
  EXPECT_EQ(lhs.after, rhs.after);
  EXPECT_EQ(lhs.labels, rhs.labels);
  EXPECT_EQ(lhs.dependencies, rhs.dependencies);
  EXPECT_EQ(lhs.requirements, rhs.requirements);
}

export void expect_eq(const data::tstate &lhs, const data::tstate &rhs) {

  ASSERT_EQ(lhs.labels.size(), rhs.labels.size());
  for (auto e : std::views::zip(lhs.labels, rhs.labels))
    expect_eq(std::get<0>(e), std::get<1>(e));

  ASSERT_EQ(lhs.projects.size(), rhs.projects.size());
  for (auto e : std::views::zip(lhs.projects, rhs.projects))
    expect_eq(std::get<0>(e), std::get<1>(e));

  ASSERT_EQ(lhs.groups.size(), rhs.groups.size());
  for (auto e : std::views::zip(lhs.groups, rhs.groups))
    expect_eq(std::get<0>(e), std::get<1>(e));

  ASSERT_EQ(lhs.tasks.size(), rhs.tasks.size());
  for (auto e : std::views::zip(lhs.tasks, rhs.tasks))
    expect_eq(std::get<0>(e), std::get<1>(e));
}
