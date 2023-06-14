export module ut_helpers;

import data;

import boost.ut;

import std;

export std::string format(const data::tparse_error &error) {
  return std::format(R"(Parsing error.
line    {}: {}
message {}
)",
                     error.line_no, error.line, error.message);
}

// Boost UT requires the operator<< to be in the namespace of the argument.
namespace data {
std::ostream &operator<<(std::ostream &os, tcolor color) {
  return os << static_cast<int>(color);
}
std::ostream &operator<<(std::ostream &os, ttask::tstatus status) {
  return os << static_cast<int>(status);
}
} // namespace data

// Violates [namespace.std]
namespace std {
ostream &operator<<(ostream &os, optional<chrono::year_month_day> date) {
  if (!date)
    return os << "[]";

  return os << *date;
}
} // namespace std

export template <class T, class... Args>
void expect_true(T &&v, Args &&...args) {
  (boost::ut::expect(static_cast<bool>(v)) << ... << args);
}

export template <class T> auto assert_true(T &&v) {
  using namespace boost::ut::operators;
  return boost::ut::expect((static_cast<bool>(v)) >> boost::ut::fatal);
}

export template <class T> void expect_false(T &&v) {
  boost::ut::expect(!static_cast<bool>(v));
}

export template <class T> void assert_false(T &&v) {
  using namespace boost::ut::operators;
  boost::ut::expect((!static_cast<bool>(v)) >> boost::ut::fatal);
}

export void expect_eq(const data::tparse_error &lhs,
                      const data::tparse_error &rhs) {
  boost::ut::expect(boost::ut::eq(lhs.line_no, rhs.line_no));
  boost::ut::expect(boost::ut::eq(lhs.line, rhs.line));
  boost::ut::expect(boost::ut::eq(lhs.message, rhs.message));
}

export void expect_eq(const data::tlabel &lhs, const data::tlabel &rhs) {
  boost::ut::expect(boost::ut::eq(lhs.id, rhs.id));
  boost::ut::expect(boost::ut::eq(lhs.name, rhs.name));
  boost::ut::expect(boost::ut::eq(lhs.description, rhs.description));
  boost::ut::expect(boost::ut::eq(lhs.color, rhs.color));
}

export void expect_eq(const data::tproject &lhs, const data::tproject &rhs) {
  boost::ut::expect(boost::ut::eq(lhs.id, rhs.id));
  boost::ut::expect(boost::ut::eq(lhs.name, rhs.name));
  boost::ut::expect(boost::ut::eq(lhs.description, rhs.description));
  boost::ut::expect(boost::ut::eq(lhs.color, rhs.color));
  boost::ut::expect(boost::ut::eq(lhs.active, rhs.active));
}

export void expect_eq(const data::tgroup &lhs, const data::tgroup &rhs) {
  boost::ut::expect(boost::ut::eq(lhs.id, rhs.id));
  boost::ut::expect(boost::ut::eq(lhs.project, rhs.project));
  boost::ut::expect(boost::ut::eq(lhs.name, rhs.name));
  boost::ut::expect(boost::ut::eq(lhs.description, rhs.description));
  boost::ut::expect(boost::ut::eq(lhs.color, rhs.color));
  boost::ut::expect(boost::ut::eq(lhs.active, rhs.active));
}

export void expect_eq(const data::ttask &lhs, const data::ttask &rhs) {
  boost::ut::expect(boost::ut::eq(lhs.id, rhs.id));
  boost::ut::expect(boost::ut::eq(lhs.project, rhs.project));
  boost::ut::expect(boost::ut::eq(lhs.group, rhs.group));
  boost::ut::expect(boost::ut::eq(lhs.title, rhs.title));
  boost::ut::expect(boost::ut::eq(lhs.description, rhs.description));
  boost::ut::expect(boost::ut::eq(lhs.status, rhs.status));
  boost::ut::expect(boost::ut::eq(lhs.after, rhs.after));
  boost::ut::expect(boost::ut::eq(lhs.labels, rhs.labels));
  boost::ut::expect(boost::ut::eq(lhs.dependencies, rhs.dependencies));
  boost::ut::expect(boost::ut::eq(lhs.requirements, rhs.requirements));
}

export void expect_eq(const data::tstate &lhs, const data::tstate &rhs) {
  using namespace boost::ut::operators;
  boost::ut::expect((boost::ut::eq(lhs.labels.size(), rhs.labels.size())) >>
                    boost::ut::fatal);

  for (auto e : std::views::zip(lhs.labels, rhs.labels))
    expect_eq(std::get<0>(e), std::get<1>(e));

  boost::ut::expect((boost::ut::eq(lhs.projects.size(), rhs.projects.size())) >>
                    boost::ut::fatal);
  for (auto e : std::views::zip(lhs.projects, rhs.projects))
    expect_eq(std::get<0>(e), std::get<1>(e));

  boost::ut::expect((boost::ut::eq(lhs.groups.size(), rhs.groups.size())) >>
                    boost::ut::fatal);
  for (auto e : std::views::zip(lhs.groups, rhs.groups))
    expect_eq(std::get<0>(e), std::get<1>(e));

  boost::ut::expect((boost::ut::eq(lhs.tasks.size(), rhs.tasks.size())) >>
                    boost::ut::fatal);
  for (auto e : std::views::zip(lhs.tasks, rhs.tasks))
    expect_eq(std::get<0>(e), std::get<1>(e));
}
