#include <expected>

#include <gtest/gtest.h>

import helpers;
import data;
import stl;

// line numbers for multiline and afterwards
// implement colors.

TEST(parser, empty) {
  std::string_view input;

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  EXPECT_TRUE(result);
  expect_eq(**result, data::tstate{});
}

TEST(parser, empty_lines) {
  std::string_view input = R"(




)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  EXPECT_TRUE(result);
  expect_eq(**result, data::tstate{});
}

TEST(parser, unknown_header) {
  std::string_view input = "[not a valid header name]";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{1, input, "found unknown header"});
}

TEST(parser, pair_outside_header) {
  std::string_view input = "key=value";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{1, input, "value is not attached to a header"});
}

TEST(parser, invalid_line) {
  std::string_view input = "key equals value";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{1, input, "value contains no '=' separator"});
}

TEST(parser, multiline_at_eof) {
  std::string_view input = "key=<<<";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{1, input,
                               "value ends with start of multiline marker"});
}

TEST(parser, multiline_no_terminator) {
  std::string_view input = R"(key=<<<



)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{
                1, input, "end of file before multiline terminator was found"});
}

TEST(parser, multiline_line_count) {
  std::string_view input = R"(
[task]
id=1
title=hello
description=<<<
Hello
>>>

[task]
ID=2
)";

  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(*result.error(),
            data::tparse_error{10, "ID=2", "invalid field name"});
}
