import ut_helpers;

import data;

import boost.ut;

import std;

namespace {

using namespace boost::ut::literals;

boost::ut::suite<"parser_task"> suite = [] {
  "id_missing"_test = [] {
    std::string_view input = R"(
[task])";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{2, "", "missing mandatory field »id«"});
  };

  "id_zero"_test = [] {
    std::string_view input = R"(
[task]
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
[task]
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
[task]
id=a)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{3, "a", "invalid number for field »id«"});
  };

  "id_number_and_garbage"_test = [] {
    std::string_view input = R"(
[task]
id=0a)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{3, "0a",
                                 "number contains non-digits for field »id«"});
  };

  "project_empty"_test = [] {
    std::string_view input = R"(
[task]
id=1
project=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{4, "", "invalid number for field »project«"});
  };

  "project_zero"_test = [] {
    std::string_view input = R"(
[task]
id=1
project=0
title=abc
)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result, data::tstate{.tasks = {data::ttask{1, 0, 0, "abc"}}});
  };

  "project_does_not_exist"_test = [] {
    std::string_view input = R"(
[task]
id=1
project=1)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{
            2, "", "id field »project« has no linked record for value »1«"});
  };

  "project_duplicate"_test = [] {
    std::string_view input = R"(
[project]
id=1
name=test

[task]
id=1
project=1
project=1)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{9, "1", "duplicate entry for field »project«"});
  };

  "project_not_a_number"_test = [] {
    std::string_view input = R"(
[task]
id=1
project=a)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{4, "a", "invalid number for field »project«"});
  };

  "project_number_and_garbage"_test = [] {
    std::string_view input = R"(
[task]
id=1
project=0a)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{
                  4, "0a", "number contains non-digits for field »project«"});
  };

  "group_empty"_test = [] {
    std::string_view input = R"(
[task]
id=1
group=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{4, "", "invalid number for field »group«"});
  };

  "group_zero"_test = [] {
    std::string_view input = R"(
[task]
id=1
group=0
title=abc
)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result, data::tstate{.tasks = {data::ttask{1, 0, 0, "abc"}}});
  };

  "group_does_not_exist"_test = [] {
    std::string_view input = R"(
[task]
id=1
group=1)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{
            2, "", "id field »group« has no linked record for value »1«"});
  };

  "group_duplicate"_test = [] {
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

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{14, "99", "duplicate entry for field »group«"});
  };

  "group_not_a_number"_test = [] {
    std::string_view input = R"(
[task]
id=1
group=a)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{4, "a", "invalid number for field »group«"});
  };

  "group_number_and_garbage"_test = [] {
    std::string_view input = R"(
[task]
id=1
group=0a)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{
                  4, "0a", "number contains non-digits for field »group«"});
  };

  "project_zero_group_zero"_test = [] {
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

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result,
              data::tstate{.projects = {data::tproject{42, "test project"}},
                           .groups = {data::tgroup{99, 42, "test group"}},
                           .tasks = {data::ttask{1, 0, 0, "abc"}}});
  };

  "project_zero_group_set"_test = [] {
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

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result,
              data::tstate{.projects = {data::tproject{42, "test project"}},
                           .groups = {data::tgroup{99, 42, "test group"}},
                           .tasks = {data::ttask{1, 0, 99, "abc"}}});
  };

  "project_set_group_zero"_test = [] {
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

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result,
              data::tstate{.projects = {data::tproject{42, "test project"}},
                           .groups = {data::tgroup{99, 42, "test group"}},
                           .tasks = {data::ttask{1, 42, 0, "abc"}}});
  };

  "project_set_group_set"_test = [] {
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

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{
                  11, "", "task »1« has both a »group« and a »project« set"});
  };

  "title_missing"_test = [] {
    std::string_view input = R"(
[task]
id=1)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{2, "", "missing mandatory field »title«"});
  };

  "title_empty"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{4, "",
                                 "an empty string is not a valid value for "
                                 "mandatory string field »title«"});
  };

  "title_duplicate"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
title=def)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{5, "def", "duplicate entry for field »title«"});
  };

  "description_empty"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
description=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result, data::tstate{.tasks = {data::ttask{1, 0, 0, "abc"}}});
  };

  "description_duplicate"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
description=def
description=ghi)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{6, "ghi",
                                 "duplicate entry for field »description«"});
  };

  "status_empty"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
status=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{5, "", "invalid status value for field »status«"});
  };

  "status_invalid"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
status=not a status)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "not a status",
                                 "invalid status value for field »status«"});
  };

  "status_duplicate"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
status=backlog
status=selected)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{6, "selected",
                                 "duplicate entry for field »status«"});
  };

  "status_backlog"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
status=backlog)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(
        **result,
        data::tstate{.tasks = {data::ttask{1, 0, 0, "abc", "",
                                           data::ttask::tstatus::backlog}}});
  };

  "status_selected"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
status=selected)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(
        **result,
        data::tstate{.tasks = {data::ttask{1, 0, 0, "abc", "",
                                           data::ttask::tstatus::selected}}});
  };

  "status_progress"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
status=progress)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(
        **result,
        data::tstate{.tasks = {data::ttask{1, 0, 0, "abc", "",
                                           data::ttask::tstatus::progress}}});
  };

  "status_review"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
status=review)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(
        **result,
        data::tstate{.tasks = {data::ttask{1, 0, 0, "abc", "",
                                           data::ttask::tstatus::review}}});
  };

  "status_done"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
status=done)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result,
              data::tstate{.tasks = {data::ttask{1, 0, 0, "abc", "",
                                                 data::ttask::tstatus::done}}});
  };

  "status_discarded"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
status=discarded)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(
        **result,
        data::tstate{.tasks = {data::ttask{1, 0, 0, "abc", "",
                                           data::ttask::tstatus::discarded}}});
  };

  "after_empty"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
after=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{
                  5, "", "month separator not found for field »after«"});
  };

  "after_no_month_separator"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
after=no month separator)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{5, "no month separator",
                           "month separator not found for field »after«"});
  };

  "after_year_not_a_number"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
after=a.)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "a.", "invalid year for field »after«"});
  };

  "after_year_and_garbage"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
after=1999a.)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "1999a.",
                                 "year contains non-digits for field »after«"});
  };

  "after_no_day_separator"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
after=1999.)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "1999.",
                                 "day separator not found for field »after«"});
  };

  "after_month_not_a_number"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
after=1999.a.)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{5, "1999.a.", "invalid month for field »after«"});
  };

  "after_month_and_garbage"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
after=1999.2a.)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{5, "1999.2a.",
                           "month contains non-digits for field »after«"});
  };

  "after_day_not_a_number"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
after=1999.02.a)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{5, "1999.02.a", "invalid day for field »after«"});
  };

  "after_day_and_garbage"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
after=1999.02.29a)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "1999.02.29a",
                                 "day contains non-digits for field »after«"});
  };

  "after_invalid_date"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
after=1999.02.29)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "1999.02.29",
                                 "not a valid date for field »after«"});
  };

  "after_duplicate"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
after=1999.02.28
after=1970.01.01)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{6, "1970.01.01",
                                 "duplicate entry for field »after«"});
  };

  "after_labels_empty"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
labels=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result, data::tstate{.tasks = {data::ttask{1, 0, 0, "abc"}}});
  };

  "after_labels_valid"_test = [] {
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

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result,
              data::tstate{
                  .labels = {data::tlabel{10, "abc"}, data::tlabel{15, "def"},
                             data::tlabel{20, "ghi"}},
                  .tasks = {data::ttask{
                      1, 0, 0, "abc", "", data::ttask::tstatus::backlog,
                      std::optional<std::chrono::year_month_day>{},
                      std::vector<std::size_t>{10, 15, 20}}}});
  };

  "after_labels_not_a_number"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
labels=10,a,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "10,a,    20",
                                 "invalid number for field »labels«"});
  };

  "after_labels_zero_length_number"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
labels=10,,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "10,,    20",
                                 "invalid number for field »labels«"});
  };

  "after_labels_number_and_garbage"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
labels=10,0a,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{5, "10,0a,    20",
                           "number contains non-digits for field »labels«"});
  };

  "after_labels_zero"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
labels=10,0,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{
                  5, "10,0,    20",
                  "zero is not a valid value for an id list field »labels«"});
  };

  "labels_duplicate"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
labels=10
labels=42,99)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{6, "42,99", "duplicate entry for field »labels«"});
  };

  "after_dependencies_empty"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
dependencies=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result, data::tstate{.tasks = {data::ttask{1, 0, 0, "abc"}}});
  };

  "after_dependencies_valid"_test = [] {
    // note that task dependencies are not validated
    // this due to unresolvable circular dependencies.
    // TODO the final parse state should resolve these dependencies and flag
    // circular dependencies.
    std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10,15,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(
        **result,
        data::tstate{.tasks = {data::ttask{
                         1, 0, 0, "abc", "", data::ttask::tstatus::backlog,
                         std::optional<std::chrono::year_month_day>{},
                         std::vector<std::size_t>{},
                         std::vector<std::size_t>{10, 15, 20}}}});
  };

  "after_dependencies_not_a_number"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10,a,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "10,a,    20",
                                 "invalid number for field »dependencies«"});
  };

  "after_dependencies_zero_length_number"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10,,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "10,,    20",
                                 "invalid number for field »dependencies«"});
  };

  "after_dependencies_number_and_garbage"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10,0a,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{
                  5, "10,0a,    20",
                  "number contains non-digits for field »dependencies«"});
  };

  "after_dependencies_zero"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10,0,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{
            5, "10,0,    20",
            "zero is not a valid value for an id list field »dependencies«"});
  };

  "dependencies_duplicate"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
dependencies=10
dependencies=42,99)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{6, "42,99",
                                 "duplicate entry for field »dependencies«"});
  };

  "after_requirements_empty"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
requirements=)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result, data::tstate{.tasks = {data::ttask{1, 0, 0, "abc"}}});
  };

  "after_requirements_valid"_test = [] {
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

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(
        **result,
        data::tstate{.projects = {data::tproject{1, "project"}},
                     .groups = {data::tgroup{10, 1, "abc"}, //
                                data::tgroup{15, 1, "def"}, //
                                data::tgroup{20, 1, "ghi"}},
                     .tasks = {data::ttask{
                         1, 0, 0, "abc", "", data::ttask::tstatus::backlog,
                         std::optional<std::chrono::year_month_day>{},
                         std::vector<std::size_t>{}, std::vector<std::size_t>{},
                         std::vector<std::size_t>{10, 15, 20}}}});
  };

  "after_requirements_not_a_number"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
requirements=10,a,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "10,a,    20",
                                 "invalid number for field »requirements«"});
  };

  "after_requirements_zero_length_number"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
requirements=10,,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{5, "10,,    20",
                                 "invalid number for field »requirements«"});
  };

  "after_requirements_number_and_garbage"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
requirements=10,0a,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{
                  5, "10,0a,    20",
                  "number contains non-digits for field »requirements«"});
  };

  "after_requirements_zero"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
requirements=10,0,    20)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(
        result.error(),
        data::tparse_error{
            5, "10,0,    20",
            "zero is not a valid value for an id list field »requirements«"});
  };

  "requirements_duplicate"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc
requirements=10
requirements=42,99)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    assert_false(result);
    expect_eq(result.error(),
              data::tparse_error{6, "42,99",
                                 "duplicate entry for field »requirements«"});
  };

  "minimal_valid"_test = [] {
    std::string_view input = R"(
[task]
id=1
title=abc)";

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result, data::tstate{.tasks = {data::ttask{1, 0, 0, "abc"}}});
  };

  "all_fields_except_group"_test = [] {
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

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result,
              data::tstate{
                  .labels =
                      {
                          data::tlabel{2, "xxx"},
                          data::tlabel{4, "yyy"},
                          data::tlabel{8, "zzz"},
                      },
                  .projects = {data::tproject{42, "answer"},
                               data::tproject{99, "nice number"}},
                  .groups = {data::tgroup{10, 42, "abc"}, //
                             data::tgroup{15, 99, "def"}, //
                             data::tgroup{20, 42, "ghi"}},
                  .tasks = {data::ttask{
                      1, 42, 0, "abc", "message", data::ttask::tstatus::review,
                      std::optional<std::chrono::year_month_day>{
                          std::chrono::year_month_day{std::chrono::year{2000},
                                                      std::chrono::month{1},
                                                      std::chrono::day{1}}},
                      std::vector<std::size_t>{2, 8, 4},
                      std::vector<std::size_t>{2, 3, 5, 7},
                      std::vector<std::size_t>{10, 20, 15}}}});
  };

  "all_fields_except_project"_test = [] {
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

    std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
        data::parse(input);

    expect_true(result) << [&] { return format(result.error()); }
                        << boost::ut::fatal;
    expect_eq(**result,
              data::tstate{
                  .labels =
                      {
                          data::tlabel{2, "xxx"},
                          data::tlabel{4, "yyy"},
                          data::tlabel{8, "zzz"},
                      },
                  .projects = {data::tproject{42, "answer"},
                               data::tproject{99, "nice number"}},
                  .groups = {data::tgroup{10, 42, "abc"}, //
                             data::tgroup{15, 99, "def"}, //
                             data::tgroup{20, 42, "ghi"}},
                  .tasks = {data::ttask{
                      1, 0, 15, "abc", "message", data::ttask::tstatus::review,
                      std::optional<std::chrono::year_month_day>{
                          std::chrono::year_month_day{std::chrono::year{2000},
                                                      std::chrono::month{1},
                                                      std::chrono::day{1}}},
                      std::vector<std::size_t>{2, 8, 4},
                      std::vector<std::size_t>{2, 3, 5, 7},
                      std::vector<std::size_t>{10, 20, 15}}}});
  };
};
}
