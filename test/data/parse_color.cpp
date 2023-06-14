import ut_helpers;

import data;

import boost.ut;

import std;

namespace {

using namespace boost::ut::literals;

// A label is the easiest to validate the parsed colors.

void valid(std::string name, data::tcolor color) {
  std::string input = R"(
[label]
id=1
name=a
color=)" + name;

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  expect_true(result) << [&] { return format(result.error()); }
                      << boost::ut::fatal;
  expect_eq(**result, data::tstate{.labels = {data::tlabel{
                                       .id = 1, .name = "a", .color = color}}});
}

void invalid(std::string name) {
  std::string input = R"(
[label]
id=1
name=a
color=)" + name;

  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);

  assert_false(result);
  expect_eq(
      result.error(),
      data::tparse_error{5, name, "invalid color value for field »color«"});
}

boost::ut::suite<"parser_color"> suite = [] {
  "all_valid"_test = [] {
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
  };

  "all_invalid"_test = [] {
    invalid("");
    invalid("grain");
    invalid("purple");
    invalid("pink");
    invalid("teal");
  };
};
} // namespace
