#include <gtest/gtest.h>

import helpers;
import data;
import stl;

TEST(parser_group, id_missing) {
  std::string_view input = R"(
[group])";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{2, "", "missing mandatory field »id«"});
}

TEST(parser_group, id_zero) {
  std::string_view input = R"(
[group]
id=0)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      result.error(),
      data::tparse_error{
          3, "0", "zero is not a valid value for mandatory id field »id«"});
}

TEST(parser_group, id_duplicate) {
  std::string_view input = R"(
[group]
id=1
id=1)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{4, "1", "duplicate entry for field »id«"});
}

TEST(parser_group, id_not_a_number) {
  std::string_view input = R"(
[group]
id=a)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{3, "a", "invalid number for field »id«"});
}

TEST(parser_group, id_number_and_garbage) {
  std::string_view input = R"(
[group]
id=0a)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      result.error(),
      data::tparse_error{3, "0a", "number contains non-digits for field »id«"});
}

TEST(parser_group, project_missing) {
  std::string_view input = R"(
[group]
id=1)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{2, "", "missing mandatory field »project«"});
}

TEST(parser_group, project_zero) {
  std::string_view input = R"(
[group]
id=1
project=0)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{
                4, "0",
                "zero is not a valid value for mandatory id field »project«"});
}

TEST(parser_group, project_does_not_exist) {
  std::string_view input = R"(
[group]
id=1
project=1)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      result.error(),
      data::tparse_error{
          2, "", "id field »project« has no linked record for value »1«"});
}

TEST(parser_group, project_duplicate) {
  std::string_view input = R"(
[project]
id=1
name=test

[group]
id=1
project=1
project=1)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{9, "1", "duplicate entry for field »project«"});
}

TEST(parser_group, project_not_a_number) {
  std::string_view input = R"(
[group]
id=1
project=a)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{4, "a", "invalid number for field »project«"});
}

TEST(parser_group, project_number_and_garbage) {
  std::string_view input = R"(
[group]
id=1
project=0a)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{
                4, "0a", "number contains non-digits for field »project«"});
}

TEST(parser_group, name_missing) {
  std::string_view input = R"(
[project]
id=1
name=test

[group]
id=1
project=1)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{6, "", "missing mandatory field »name«"});
}

TEST(parser_group, name_empty) {
  std::string_view input = R"(
[project]
id=1
name=test

[group]
id=1
project=1
name=)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{9, "",
                               "an empty string is not a valid value for "
                               "mandatory string field »name«"});
}

TEST(parser_group, name_duplicate) {
  std::string_view input = R"(
[project]
id=1
name=test

[group]
id=1
project=1
name=abc
name=def)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{10, "def", "duplicate entry for field »name«"});
}

TEST(parser_group, description_empty) {
  std::string_view input = R"(
[project]
id=42
name=test

[group]
id=1
project=42
name=abc
description=)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(result.error());
  expect_eq(**result, data::tstate{.projects = {data::tproject{42, "test"}},
                                   .groups = {data::tgroup{1, 42, "abc"}}});
}

TEST(parser_group, description_duplicate) {
  std::string_view input = R"(
[project]
id=1
name=test

[group]
id=1
project=1
name=abc
description=This is the first description.
description=This is the second description.
)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{11, "This is the second description.",
                               "duplicate entry for field »description«"});
}

TEST(parser_group, color_empty) {
  std::string_view input = R"(
[project]
id=42
name=test

[group]
id=1
project=42
name=abc
description=def
color=)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      result.error(),
      data::tparse_error{11, "", "invalid color value for field »color«"});
}

TEST(parser_group, color_duplicate) {
  std::string_view input = R"(
[project]
id=1
name=test

[group]
id=1
project=1
name=abc
description=def
color=red
color=green
)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      result.error(),
      data::tparse_error{12, "green", "duplicate entry for field »color«"});
}

TEST(parser_group, boolean_empty) {
  std::string_view input = R"(
[project]
id=1
name=test

[group]
id=1
project=1
name=abc
description=def
color=green
active=)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      result.error(),
      data::tparse_error{12, "", "invalid boolean value for field »active«"});
}

TEST(parser_group, boolean_invalid_value) {
  std::string_view input = R"(
[project]
id=1
name=test

[group]
id=1
project=1
name=abc
description=def
color=green
active=not a boolean value)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(result.error(),
            data::tparse_error{12, "not a boolean value",
                               "invalid boolean value for field »active«"});
}

TEST(parser_group, boolean_duplicate) {
  std::string_view input = R"(
[project]
id=1
name=test

[group]
id=1
project=1
name=abc
description=def
color=green
active=true
active=false)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      result.error(),
      data::tparse_error{13, "false", "duplicate entry for field »active«"});
}

TEST(parser_group, minimal_valid) {
  std::string_view input = R"(
[project]
id=42
name=test

[group]
id=1
project=42
name=foo)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(result.error());
  expect_eq(**result, data::tstate{.projects = {data::tproject{42, "test"}},
                                   .groups = {data::tgroup{1, 42, "foo"}}});
}

TEST(parser_group, all_fields) {
  std::string_view input = R"(
[project]
id=42
name=test

[group]
id=1
project=42
name=foo
description=bar
color=red
active=false
)";

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(result.error());
  expect_eq(
      **result,
      data::tstate{.projects = {data::tproject{42, "test"}},
                   .groups = {data::tgroup{1, 42, "foo", "bar",
                                           data::tcolor::light_red, false}}});
}
