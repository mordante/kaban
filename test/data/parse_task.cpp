#include <expected>

#include <gtest/gtest.h>

import helpers;
import data;
import stl;

TEST(parser_task, id_missing) {
  std::string_view input = R"(
[task])";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{2, "", "missing mandatory field »id«"});
}

TEST(parser_task, id_zero) {
  std::string_view input = R"(
[task]
id=0)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{
          3, "0", "zero is not a valid value for mandatory id field »id«"});
}

TEST(parser_task, id_duplicate) {
  std::string_view input = R"(
[task]
id=1
id=1)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{4, "1", "duplicate entry for field »id«"});
}

TEST(parser_task, id_not_a_number) {
  std::string_view input = R"(
[task]
id=a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{3, "a", "invalid number for field »id«"});
}

TEST(parser_task, id_number_and_garbage) {
  std::string_view input = R"(
[task]
id=0a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{3, "0a", "number contains non-digits for field »id«"});
}

TEST(parser_task, project_empty) {
  std::string_view input = R"(
[task]
id=1
project=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{4, "", "invalid number for field »project«"});
}

TEST(parser_task, project_zero) {
  std::string_view input = R"(
[task]
id=1
project=0
title=abc
)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc"}}});
}

TEST(parser_task, project_does_not_exist) {
  std::string_view input = R"(
[task]
id=1
project=1)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{
          2, "", "id field »project« has no linked record for value »1«"});
}

TEST(parser_task, project_duplicate) {
  std::string_view input = R"(
[project]
id=1
name=test

[task]
id=1
project=1
project=1)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{9, "1", "duplicate entry for field »project«"});
}

TEST(parser_task, project_not_a_number) {
  std::string_view input = R"(
[task]
id=1
project=a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{4, "a", "invalid number for field »project«"});
}

TEST(parser_task, project_number_and_garbage) {
  std::string_view input = R"(
[task]
id=1
project=0a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{
                4, "0a", "number contains non-digits for field »project«"});
}

TEST(parser_task, group_empty) {
  std::string_view input = R"(
[task]
id=1
group=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{4, "", "invalid number for field »group«"});
}

TEST(parser_task, group_zero) {
  std::string_view input = R"(
[task]
id=1
group=0
title=abc
)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc"}}});
}

TEST(parser_task, group_does_not_exist) {
  std::string_view input = R"(
[task]
id=1
group=1)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{
                2, "", "id field »group« has no linked record for value »1«"});
}

TEST(parser_task, group_duplicate) {
  std::string_view input = R"(
[project]
id=42
name=test project

[group]
id=99
project=42
name=test group

[task]
id=1
group=99
group=99)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{14, "99", "duplicate entry for field »group«"});
}

TEST(parser_task, group_not_a_number) {
  std::string_view input = R"(
[task]
id=1
group=a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{4, "a", "invalid number for field »group«"});
}

TEST(parser_task, group_number_and_garbage) {
  std::string_view input = R"(
[task]
id=1
group=0a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{4, "0a",
                               "number contains non-digits for field »group«"});
}

TEST(parser_task, project_zero_group_zero) {
  std::string_view input = R"(
[project]
id=42
name=test project

[group]
id=99
project=42
name=test group

[task]
id=1
project=0
group=0
title=abc)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.projects = {project{42, "test project"}},
                                   .groups = {group{99, 42, "test group"}},
                                   .tasks = {task{1, 0, 0, "abc"}}});
}

TEST(parser_task, project_zero_group_set) {
  std::string_view input = R"(
[project]
id=42
name=test project

[group]
id=99
project=42
name=test group

[task]
id=1
project=0
group=99
title=abc)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.projects = {project{42, "test project"}},
                                   .groups = {group{99, 42, "test group"}},
                                   .tasks = {task{1, 0, 99, "abc"}}});
}

TEST(parser_task, project_set_group_zero) {
  std::string_view input = R"(
[project]
id=42
name=test project

[group]
id=99
project=42
name=test group

[task]
id=1
project=42
group=0
title=abc)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.projects = {project{42, "test project"}},
                                   .groups = {group{99, 42, "test group"}},
                                   .tasks = {task{1, 42, 0, "abc"}}});
}

TEST(parser_task, project_set_group_set) {
  std::string_view input = R"(
[project]
id=42
name=test project

[group]
id=99
project=42
name=test group

[task]
id=1
project=42
group=99
title=abc)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{
                11, "", "task »1« has both a »group« and a »project« set"});
}

TEST(parser_task, title_missing) {
  std::string_view input = R"(
[task]
id=1)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{2, "", "missing mandatory field »title«"});
}

TEST(parser_task, title_empty) {
  std::string_view input = R"(
[task]
id=1
title=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{4, "",
                               "an empty string is not a valid value for "
                               "mandatory string field »title«"});
}

TEST(parser_task, title_duplicate) {
  std::string_view input = R"(
[task]
id=1
title=abc
title=def)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "def", "duplicate entry for field »title«"});
}

TEST(parser_task, description_empty) {
  std::string_view input = R"(
[task]
id=1
title=abc
description=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc"}}});
}

TEST(parser_task, description_duplicate) {
  std::string_view input = R"(
[task]
id=1
title=abc
description=def
description=ghi)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{6, "ghi", "duplicate entry for field »description«"});
}

TEST(parser_task, status_empty) {
  std::string_view input = R"(
[task]
id=1
title=abc
status=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{5, "", "invalid status value for field »status«"});
}

TEST(parser_task, status_invalid) {
  std::string_view input = R"(
[task]
id=1
title=abc
status=not a status)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "not a status",
                               "invalid status value for field »status«"});
}

TEST(parser_task, status_duplicate) {
  std::string_view input = R"(
[task]
id=1
title=abc
status=backlog
status=selected)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{6, "selected", "duplicate entry for field »status«"});
}

TEST(parser_task, status_backlog) {
  std::string_view input = R"(
[task]
id=1
title=abc
status=backlog)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc", "",
                                                  task::tstatus::backlog}}});
}

TEST(parser_task, status_selected) {
  std::string_view input = R"(
[task]
id=1
title=abc
status=selected)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc", "",
                                                  task::tstatus::selected}}});
}

TEST(parser_task, status_progress) {
  std::string_view input = R"(
[task]
id=1
title=abc
status=progress)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc", "",
                                                  task::tstatus::progress}}});
}

TEST(parser_task, status_review) {
  std::string_view input = R"(
[task]
id=1
title=abc
status=review)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc", "",
                                                  task::tstatus::review}}});
}

TEST(parser_task, status_done) {
  std::string_view input = R"(
[task]
id=1
title=abc
status=done)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc", "",
                                                  task::tstatus::done}}});
}

TEST(parser_task, status_discarded) {
  std::string_view input = R"(
[task]
id=1
title=abc
status=discarded)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc", "",
                                                  task::tstatus::discarded}}});
}

TEST(parser_task, after_empty) {
  std::string_view input = R"(
[task]
id=1
title=abc
after=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{5, "", "month separator not found for field »after«"});
}

TEST(parser_task, after_no_month_separator) {
  std::string_view input = R"(
[task]
id=1
title=abc
after=no month separator)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "no month separator",
                               "month separator not found for field »after«"});
}

TEST(parser_task, after_year_not_a_number) {
  std::string_view input = R"(
[task]
id=1
title=abc
after=a.)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "a.", "invalid year for field »after«"});
}

TEST(parser_task, after_year_and_garbage) {
  std::string_view input = R"(
[task]
id=1
title=abc
after=1999a.)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "1999a.",
                               "year contains non-digits for field »after«"});
}

TEST(parser_task, after_no_day_separator) {
  std::string_view input = R"(
[task]
id=1
title=abc
after=1999.)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "1999.",
                               "day separator not found for field »after«"});
}

TEST(parser_task, after_month_not_a_number) {
  std::string_view input = R"(
[task]
id=1
title=abc
after=1999.a.)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{5, "1999.a.", "invalid month for field »after«"});
}

TEST(parser_task, after_month_and_garbage) {
  std::string_view input = R"(
[task]
id=1
title=abc
after=1999.2a.)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "1999.2a.",
                               "month contains non-digits for field »after«"});
}

TEST(parser_task, after_day_not_a_number) {
  std::string_view input = R"(
[task]
id=1
title=abc
after=1999.02.a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{5, "1999.02.a", "invalid day for field »after«"});
}

TEST(parser_task, after_day_and_garbage) {
  std::string_view input = R"(
[task]
id=1
title=abc
after=1999.02.29a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "1999.02.29a",
                               "day contains non-digits for field »after«"});
}

TEST(parser_task, after_invalid_date) {
  std::string_view input = R"(
[task]
id=1
title=abc
after=1999.02.29)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "1999.02.29",
                               "not a valid date for field »after«"});
}

TEST(parser_task, after_duplicate) {
  std::string_view input = R"(
[task]
id=1
title=abc
after=1999.02.28
after=1970.01.01)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{6, "1970.01.01", "duplicate entry for field »after«"});
}

TEST(parser_task, after_labels_empty) {
  std::string_view input = R"(
[task]
id=1
title=abc
labels=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc"}}});
}

TEST(parser_task, after_labels_valid) {
  std::string_view input = R"(
[label]
id=10
name=abc

[label]
id=15
name=def

[label]
id=20
name=ghi

[task]
id=1
title=abc
labels=10,15,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(
      **result,
      data::tstate{
          .labels = {label{10, "abc"}, label{15, "def"}, label{20, "ghi"}},
          .tasks = {task{1, 0, 0, "abc", "", task::tstatus::backlog,
                         std::optional<std::chrono::year_month_day>{},
                         std::vector<std::size_t>{10, 15, 20}}}});
}

TEST(parser_task, after_labels_not_a_number) {
  std::string_view input = R"(
[task]
id=1
title=abc
labels=10,a,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "10,a,    20",
                               "invalid number for field »labels«"});
}

TEST(parser_task, after_labels_zero_length_number) {
  std::string_view input = R"(
[task]
id=1
title=abc
labels=10,,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{5, "10,,    20", "invalid number for field »labels«"});
}

TEST(parser_task, after_labels_number_and_garbage) {
  std::string_view input = R"(
[task]
id=1
title=abc
labels=10,0a,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{5, "10,0a,    20",
                         "number contains non-digits for field »labels«"});
}

TEST(parser_task, after_labels_zero) {
  std::string_view input = R"(
[task]
id=1
title=abc
labels=10,0,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{
                5, "10,0,    20",
                "zero is not a valid value for an id list field »labels«"});
}

TEST(parser_task, labels_duplicate) {
  std::string_view input = R"(
[task]
id=1
title=abc
labels=10
labels=42,99)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{6, "42,99", "duplicate entry for field »labels«"});
}

TEST(parser_task, after_dependencies_empty) {
  std::string_view input = R"(
[task]
id=1
title=abc
dependencies=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc"}}});
}

TEST(parser_task, after_dependencies_valid) {
  // note that task dependencies are not validated
  // this due to unresolvable circular dependencies.
  // TODO the final parse state should resolve these dependencies and flag
  // circular dependencies.
  std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10,15,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(
      **result,
      data::tstate{.tasks = {task{1, 0, 0, "abc", "", task::tstatus::backlog,
                                  std::optional<std::chrono::year_month_day>{},
                                  std::vector<std::size_t>{},
                                  std::vector<std::size_t>{10, 15, 20}}}});
}

TEST(parser_task, after_dependencies_not_a_number) {
  std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10,a,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "10,a,    20",
                               "invalid number for field »dependencies«"});
}

TEST(parser_task, after_dependencies_zero_length_number) {
  std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10,,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "10,,    20",
                               "invalid number for field »dependencies«"});
}

TEST(parser_task, after_dependencies_number_and_garbage) {
  std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10,0a,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{
                5, "10,0a,    20",
                "number contains non-digits for field »dependencies«"});
}

TEST(parser_task, after_dependencies_zero) {
  std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10,0,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{
          5, "10,0,    20",
          "zero is not a valid value for an id list field »dependencies«"});
}

TEST(parser_task, dependencies_duplicate) {
  std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10
dependencies=42,99)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{6, "42,99",
                               "duplicate entry for field »dependencies«"});
}

TEST(parser_task, after_requirements_empty) {
  std::string_view input = R"(
[task]
id=1
title=abc
requirements=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc"}}});
}

TEST(parser_task, after_requirements_valid) {
  std::string_view input = R"(
[project]
id=1
name=project

[group]
id=10
project=1
name=abc

[group]
id=15
project=1
name=def

[group]
id=20
project=1
name=ghi

[task]
id=1
title=abc
requirements=10,15,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(
      **result,
      data::tstate{
          .projects = {project{1, "project"}},
          .groups = {group{10, 1, "abc"}, //
                     group{15, 1, "def"}, //
                     group{20, 1, "ghi"}},
          .tasks = {task{1, 0, 0, "abc", "", task::tstatus::backlog,
                         std::optional<std::chrono::year_month_day>{},
                         std::vector<std::size_t>{}, std::vector<std::size_t>{},
                         std::vector<std::size_t>{10, 15, 20}}}});
}

TEST(parser_task, after_requirements_not_a_number) {
  std::string_view input = R"(
[task]
id=1
title=abc
requirements=10,a,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "10,a,    20",
                               "invalid number for field »requirements«"});
}

TEST(parser_task, after_requirements_zero_length_number) {
  std::string_view input = R"(
[task]
id=1
title=abc
requirements=10,,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "10,,    20",
                               "invalid number for field »requirements«"});
}

TEST(parser_task, after_requirements_number_and_garbage) {
  std::string_view input = R"(
[task]
id=1
title=abc
requirements=10,0a,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{
                5, "10,0a,    20",
                "number contains non-digits for field »requirements«"});
}

TEST(parser_task, after_requirements_zero) {
  std::string_view input = R"(
[task]
id=1
title=abc
requirements=10,0,    20)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{
          5, "10,0,    20",
          "zero is not a valid value for an id list field »requirements«"});
}

TEST(parser_task, requirements_duplicate) {
  std::string_view input = R"(
[task]
id=1
title=abc
requirements=10
requirements=42,99)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{6, "42,99",
                               "duplicate entry for field »requirements«"});
}

TEST(parser_task, minimal_valid) {
  std::string_view input = R"(
[task]
id=1
title=abc)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.tasks = {task{1, 0, 0, "abc"}}});
}

TEST(parser_task, all_fields_except_group) {
  std::string_view input = R"(
[label]
id=2
name=xxx

[label]
id=4
name=yyy

[label]
id=8
name=zzz

[project]
id=42
name=answer

[project]
id=99
name=nice number

[group]
id=10
project=42
name=abc

[group]
id=15
project=99
name=def

[group]
id=20
project=42
name=ghi

[task]
id=1
project=42
title=abc
description=message
status=review
after=2000.01.01
labels=2,8,4
dependencies=2,3,5,7
requirements=10,20,15
)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(
      **result,
      data::tstate{
          .labels =
              {
                  label{2, "xxx"},
                  label{4, "yyy"},
                  label{8, "zzz"},
              },
          .projects = {project{42, "answer"}, project{99, "nice number"}},
          .groups = {group{10, 42, "abc"}, //
                     group{15, 99, "def"}, //
                     group{20, 42, "ghi"}},
          .tasks = {task{1, 42, 0, "abc", "message", task::tstatus::review,
                         std::optional<std::chrono::year_month_day>{
                             std::chrono::year_month_day{
                                 std::chrono::year{2000}, std::chrono::month{1},
                                 std::chrono::day{1}}},
                         std::vector<std::size_t>{2, 8, 4},
                         std::vector<std::size_t>{2, 3, 5, 7},
                         std::vector<std::size_t>{10, 20, 15}}}});
}

TEST(parser_task, all_fields_except_project) {
  std::string_view input = R"(
[label]
id=2
name=xxx

[label]
id=4
name=yyy

[label]
id=8
name=zzz

[project]
id=42
name=answer

[project]
id=99
name=nice number

[group]
id=10
project=42
name=abc

[group]
id=15
project=99
name=def

[group]
id=20
project=42
name=ghi

[task]
id=1
group=15
title=abc
description=message
status=review
after=2000.01.01
labels=2,8,4
dependencies=2,3,5,7
requirements=10,20,15
)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(
      **result,
      data::tstate{
          .labels =
              {
                  label{2, "xxx"},
                  label{4, "yyy"},
                  label{8, "zzz"},
              },
          .projects = {project{42, "answer"}, project{99, "nice number"}},
          .groups = {group{10, 42, "abc"}, //
                     group{15, 99, "def"}, //
                     group{20, 42, "ghi"}},
          .tasks = {task{1, 0, 15, "abc", "message", task::tstatus::review,
                         std::optional<std::chrono::year_month_day>{
                             std::chrono::year_month_day{
                                 std::chrono::year{2000}, std::chrono::month{1},
                                 std::chrono::day{1}}},
                         std::vector<std::size_t>{2, 8, 4},
                         std::vector<std::size_t>{2, 3, 5, 7},
                         std::vector<std::size_t>{10, 20, 15}}}});
}
