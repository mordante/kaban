#include <expected>
#include <string>

#include <gtest/gtest.h>

import helpers;
import data;
import stl;

TEST(parser_label, id_missing) {
  std::string_view input = R"(
[label])";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{2, "", "missing mandatory field »id«"});
}

TEST(parser_label, id_zero) {
  std::string_view input = R"(
[label]
id=0)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{
          3, "0", "zero is not a valid value for mandatory id field »id«"});
}

TEST(parser_label, id_duplicate) {
  std::string_view input = R"(
[label]
id=1
id=1)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{4, "1", "duplicate entry for field »id«"});
}

TEST(parser_label, id_not_a_number) {
  std::string_view input = R"(
[label]
id=a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{3, "a", "invalid number for field »id«"});
}

TEST(parser_label, id_number_and_garbage) {
  std::string_view input = R"(
[label]
id=0a)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      *result.error(),
      data::tparse_error{3, "0a", "number contains non-digits for field »id«"});
}

TEST(parser_label, name_missing) {
  std::string_view input = R"(
[label]
id=1)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{2, "", "missing mandatory field »name«"});
}

TEST(parser_label, name_empty) {
  std::string_view input = R"(
[label]
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

TEST(parser_label, name_duplicate) {
  std::string_view input = R"(
[label]
id=1
name=abc
name=def)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{5, "def", "duplicate entry for field »name«"});
}

TEST(parser_label, description_empty) {
  std::string_view input = R"(
[label]
id=1
name=abc
description=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.labels = {label{1, "abc"}}});
}

TEST(parser_label, description_duplicate) {
  std::string_view input = R"(
[label]
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

TEST(parser_label, color_empty) {
  std::string_view input = R"(
[label]
id=1
name=abc
description=def
color=)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.labels = {label{1, "abc", "def"}}});
}

TEST(parser_label, color_duplicate) {
  std::string_view input = R"(
[label]
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

TEST(parser_label, minimal_valid) {
  std::string_view input = R"(
[label]
id=1
name=foo)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.labels = {label{1, "foo"}}});
}

TEST(parser_label, all_fields) {
  std::string_view input = R"(
[label]
id=1
name=foo
description=bar
color=red
)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(*result.error());
  expect_eq(**result, data::tstate{.labels = {label{1, "foo", "bar", "red"}}});
}
