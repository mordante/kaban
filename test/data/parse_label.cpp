import ut_helpers;

import data;

import boost.ut;

import std;

namespace {

using namespace boost::ut::literals;

boost::ut::suite<"parser_label"> suite = [] {
  "id_missing"_test = [] {
    std::string_view input = R"(
[label])";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{2, "", "missing mandatory field »id«"});
  };

  "id_zero"_test = [] {
    std::string_view input = R"(
[label]
id=0)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{
            3, "0", "zero is not a valid value for mandatory id field »id«"});
  };

  "id_duplicate"_test = [] {
    std::string_view input = R"(
[label]
id=1
id=1)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{4, "1", "duplicate entry for field »id«"});
  };

  "id_not_a_number"_test = [] {
    std::string_view input = R"(
[label]
id=a)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{3, "a", "invalid number for field »id«"});
  };

  "id_number_and_garbage"_test = [] {
    std::string_view input = R"(
[label]
id=0a)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{3, "0a",
                                 "number contains non-digits for field »id«"});
  };

  "name_missing"_test = [] {
    std::string_view input = R"(
[label]
id=1)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{2, "", "missing mandatory field »name«"});
  };

  "name_empty"_test = [] {
    std::string_view input = R"(
[label]
id=1
name=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{4, "",
                                 "an empty string is not a valid value for "
                                 "mandatory string field »name«"});
  };

  "name_duplicate"_test = [] {
    std::string_view input = R"(
[label]
id=1
name=abc
name=def)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "def", "duplicate entry for field »name«"});
  };

  "description_empty"_test = [] {
    std::string_view input = R"(
[label]
id=1
name=abc
description=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result, data::tstate{.labels = {data::tlabel{1, "abc"}}});
  };

  "description_duplicate"_test = [] {
    std::string_view input = R"(
[label]
id=1
name=abc
description=This is the first description.
description=This is the second description.
)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{6, "This is the second description.",
                                 "duplicate entry for field »description«"});
  };

  "color_empty"_test = [] {
    std::string_view input = R"(
[label]
id=1
name=abc
description=def
color=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{6, "", "invalid color value for field »color«"});
  };

  "color_duplicate"_test = [] {
    std::string_view input = R"(
[label]
id=1
name=abc
description=def
color=red
color=green
)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{7, "green", "duplicate entry for field »color«"});
  };

  "minimal_valid"_test = [] {
    std::string_view input = R"(
[label]
id=1
name=foo)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result, data::tstate{.labels = {data::tlabel{1, "foo"}}});
  };

  "all_fields"_test = [] {
    std::string_view input = R"(
[label]
id=1
name=foo
description=bar
color=red
)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result,
              data::tstate{.labels = {data::tlabel{1, "foo", "bar",
                                                   data::tcolor::light_red}}});
  };
};

} // namespace
