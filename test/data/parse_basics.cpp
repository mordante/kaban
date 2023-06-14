import ut_helpers;

import data;

import boost.ut;

import std;

namespace {
using namespace boost::ut::literals;

// line numbers for multiline and afterwards
// implement colors.

boost::ut::suite<"parser"> suite = [] {
  "empty"_test = [] {
    std::string_view input;

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    boost::ut::expect(bool(result));
    expect_eq(**result, data::tstate{});
  };

  "empty_lines"_test = [] {
    std::string_view input = R"(




)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result);
    expect_eq(**result, data::tstate{});
  };

  "unknown_header"_test = [] {
    std::string_view input = "[not a valid header name]";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{1, input, "found unknown header"});
  };

  "pair_outside_header"_test = [] {
    std::string_view input = "key=value";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{1, input, "value is not attached to a header"});
  };

  "invalid_line"_test = [] {
    std::string_view input = "key equals value";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{1, input, "value contains no '=' separator"});
  };

  "multiline_at_eof"_test = [] {
    std::string_view input = "key=<<<";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{1, input,
                                 "value ends with start of multiline marker"});
  };

  "multiline_no_terminator"_test = [] {
    std::string_view input = R"(key=<<<



)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{
            1, input, "end of file before multiline terminator was found"});
  };

  "multiline_line_count"_test = [] {
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

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{10, "ID=2", "invalid field name"});
  };
};
} // namespace
