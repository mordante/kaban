#include <expected>
#include <string>

#include <gtest/gtest.h>

import helpers;
import data;
import stl;

TEST(parser_project, id_missing) {
  std::string_view input = R"(
[project])";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{2, "", "missing mandatory field »id«"});
}

TEST(parser_project, id_zero) {
  std::string_view input = R"(
[project]
id=0)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{
          3, "0", "zero is not a valid value for mandatory id field »id«"});
}

TEST(parser_project, id_duplicate) {
  std::string_view input = R"(
[project]
id=1
id=1)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{4, "1", "duplicate entry for field »id«"});
}

TEST(parser_project, id_not_a_number) {
  std::string_view input = R"(
[project]
id=a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{3, "a", "invalid number for field »id«"});
}

TEST(parser_project, id_number_and_garbage) {
  std::string_view input = R"(
[project]
id=0a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{3, "0a", "number contains non-digits for field »id«"});
}

TEST(parser_project, name_missing) {
  std::string_view input = R"(
[project]
id=1)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{2, "", "missing mandatory field »name«"});
}

TEST(parser_project, name_empty) {
  std::string_view input = R"(
[project]
id=1
name=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{4, "",
                               "an empty string is not a valid value for "
                               "mandatory string field »name«"});
}

TEST(parser_project, name_duplicate) {
  std::string_view input = R"(
[project]
id=1
name=abc
name=def)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "def", "duplicate entry for field »name«"});
}

TEST(parser_project, description_empty) {
  std::string_view input = R"(
[project]
id=1
name=abc
description=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.projects = {data::tproject{1, "abc"}}});
}

TEST(parser_project, description_duplicate) {
  std::string_view input = R"(
[project]
id=1
name=abc
description=This is the first description.
description=This is the second description.
)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{6, "This is the second description.",
                               "duplicate entry for field »description«"});
}

TEST(parser_project, color_empty) {
  std::string_view input = R"(
[project]
id=1
name=abc
description=def
color=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result,
            data::tstate{.projects = {data::tproject{1, "abc", "def"}}});
}

TEST(parser_project, color_duplicate) {
  std::string_view input = R"(
[project]
id=1
name=abc
description=def
color=red
color=green
)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{7, "green", "duplicate entry for field »color«"});
}

TEST(parser_project, boolean_empty) {
  std::string_view input = R"(
[project]
id=1
name=abc
description=def
color=green
active=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{7, "", "invalid boolean value for field »active«"});
}

TEST(parser_project, boolean_invalid_value) {
  std::string_view input = R"(
[project]
id=1
name=abc
description=def
color=green
active=not a boolean value)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{7, "not a boolean value",
                               "invalid boolean value for field »active«"});
}

TEST(parser_project, boolean_duplicate) {
  std::string_view input = R"(
[project]
id=1
name=abc
description=def
color=green
active=true
active=false)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{8, "false", "duplicate entry for field »active«"});
}

TEST(parser_project, minimal_valid) {
  std::string_view input = R"(
[project]
id=1
name=foo)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.projects = {data::tproject{1, "foo"}}});
}

TEST(parser_project, all_fields) {
  std::string_view input = R"(
[project]
id=1
name=foo
description=bar
color=red
active=false
)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.projects = {data::tproject{1, "foo", "bar",
                                                               "red", false}}});
}
