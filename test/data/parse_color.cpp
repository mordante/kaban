#include <gtest/gtest.h>

import helpers;
import data;
import stl;

// A label is the easiest to validate the parsed colors.

void valid(std::string name, data::tcolor color) {
  std::string input = R"(
[label]
id=1
name=a
color=)" + name;

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_TRUE(result) << format(result.error());
  expect_eq(**result, data::tstate{.labels = {data::tlabel{
                                       .id = 1, .name = "a", .color = color}}});
}

TEST(parser_color, all_valid) {
  valid("black", data::tcolor::black);
  valid("RED", data::tcolor::red);
  valid("GREEN", data::tcolor::green);
  valid("YELLOW", data::tcolor::yellow);
  valid("BLUE", data::tcolor::blue);
  valid("MAGENTA", data::tcolor::magenta);
  valid("CYAN", data::tcolor::cyan);
  valid("gray", data::tcolor::light_gray);
  valid("GRAY", data::tcolor::dark_gray);
  valid("red", data::tcolor::light_red);
  valid("green", data::tcolor::light_green);
  valid("yellow", data::tcolor::light_yellow);
  valid("blue", data::tcolor::light_blue);
  valid("magenta", data::tcolor::light_magenta);
  valid("cyan", data::tcolor::light_cyan);
  valid("white", data::tcolor::white);
}

void invalid(std::string name) {
  std::string input = R"(
[label]
id=1
name=a
color=)" + name;

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  ASSERT_FALSE(result);
  expect_eq(
      result.error(),
      data::tparse_error{5, name, "invalid color value for field »color«"});
}

TEST(parser_color, all_invalid) {
  invalid("");
  invalid("grain");
  invalid("purple");
  invalid("pink");
  invalid("teal");
}
