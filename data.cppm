module;
#include "state.hpp"
#include <algorithm>
#include <expected>

export module data;
import stl;

export namespace data {
enum class tcolor {
  black,
  red,
  green,
  yellow,
  blue,
  magenta,
  cyan,
  light_gray,
  dark_gray,
  light_red,
  light_green,
  light_yellow,
  light_blue,
  light_magenta,
  light_cyan,
  white,
};

struct tlabel {
  std::size_t id;
  std::string name;
  std::string description;
  tcolor color;
};

struct tproject {
  std::size_t id;
  std::string name;
  std::string description;
  tcolor color;
  bool active{true};
};

struct tgroup {
  std::size_t id;
  std::size_t project{0};
  std::string name;
  std::string description;
  tcolor color;
  bool active{true};
};

struct ttask {

  enum class tstatus {
    backlog,
    selected, /**< Selected for development. */
    progress, /**< In progress. */
    review,   /**< In review. */
    done,     /**< Completed the work. */
    discarded /**< The task has been discarded. */
  };

  std::size_t id;
  std::size_t project{0};
  std::size_t group{0};
  std::string title;
  std::string description;
  tstatus status{tstatus::backlog};
  std::optional<std::chrono::year_month_day> after;
  std::vector<std::size_t> labels;
  std::vector<std::size_t> dependencies;
  std::vector<std::size_t> requirements;
};

struct tstate {
  std::vector<tlabel> labels;
  std::vector<tproject> projects;
  std::vector<tgroup> groups;
  std::vector<ttask> tasks;
};

struct tparse_error {
  int line_no;
  std::string_view line;
  std::string message;
};

} // namespace data

export namespace data {
[[nodiscard]] tstate &get_state() {
  return *static_cast<data::tstate *>(::get_state());
}

// TODO use this version, but there are memory issues when doing so.
//[[nodiscard]] std::expected<void, nullptr_t>
// void set_state(std::unique_ptr<data::tstate> &&state) {
//  return state_singleton.set(std::move(state));
void set_state(data::tstate *state) { ::set_state(state); }
} // namespace data

bool is_complete(const data::ttask &task) {
  return task.status == data::ttask::tstatus::done ||
         task.status == data::ttask::tstatus::discarded;
}

bool is_complete(const data::tgroup &group) {
  return std::ranges::all_of(data::get_state().tasks,
                             [id = group.id](const data::ttask &task) {
                               return id != task.group || is_complete(task);
                             });
}

template <class T>
const T &get_record(const std::vector<T> &range, std::size_t id) {
  auto it = std::ranges::find(range, id, &T::id);
  if (it == range.end())
    throw 42;
  return *it;
}

export namespace data {
const data::tlabel &get_label(std::size_t id) {
  return get_record(data::get_state().labels, id);
}

const data::tproject &get_project(std::size_t id) {
  return get_record(data::get_state().projects, id);
}

const data::tgroup &get_group(std::size_t id) {
  return get_record(data::get_state().groups, id);
}

const data::ttask &get_task(std::size_t id) {
  return get_record(data::get_state().tasks, id);
}

bool is_blocked(const data::ttask &task) {
  if (std::ranges::any_of(task.dependencies, [](std::size_t id) {
        return !is_complete(get_task(id));
      }))
    return true;

  if (std::ranges::any_of(task.requirements, [](std::size_t id) {
        return !is_complete(get_group(id));
      }))
    return true;

  if (!task.after)
    return false;

  return std::chrono::system_clock::now() <=
         static_cast<std::chrono::sys_days>(*task.after);
}

bool is_active(const data::ttask &task) {
  if (task.project != 0)
    return get_project(task.project).active;

  if (task.group == 0)
    return true;

  data::tgroup group = get_group(task.group);
  return group.active && get_project(group.project).active;
}

} // namespace data

class parser {
public:
  struct tresult {

    enum { eof, empty, header, pair } type;
    std::array<std::string_view, 2> data;
  };

  explicit parser(std::string_view data)
      : data_(data), next_{data.begin(), 1} {}

  std::expected<tresult, data::tparse_error> parse() {

    current_ = next_;

    if (current_.cursor_ == data_.end())
      return tresult{tresult::eof, {}};

    switch (*current_.cursor_) {
    case '\n':
      ++next_.cursor_;
      ++next_.line_;
      return tresult{tresult::empty};

    case '[':
      return parse_header();
    }

    return parse_value();
  }

  int line() const { return current_.line_; }

private:
  std::expected<tresult, data::tparse_error> parse_header() {
    auto end = std::find(current_.cursor_ + 1, data_.end(), '\n');

    next_.cursor_ = end + (end != data_.end());
    ++next_.line_;

    return tresult{tresult::header,
                   {std::string_view{current_.cursor_, end}, {}}};
  }

  std::expected<tresult, data::tparse_error> parse_value() {

    auto separator = std::find(current_.cursor_ + 1, data_.end(), '=');
    if (separator == data_.end()) {
      next_.cursor_ = data_.end();
      next_.line_ = -1;
      return std::unexpected<data::tparse_error>{
          std::in_place, current_.line_,
          std::string_view{current_.cursor_, data_.end()},
          "value contains no '=' separator"};
    }

    auto begin = separator + 1;
    auto end = std::find(begin, data_.end(), '\n');

    std::string_view value{begin, end};
    if (value == "<<<") {
      // multiline value
      if (end == data_.end()) {
        next_.cursor_ = data_.end();
        next_.line_ = -1;
        return std::unexpected<data::tparse_error>{
            std::in_place, current_.line_,
            std::string_view{current_.cursor_, data_.end()},
            "value ends with start of multiline marker"};
      }
      begin = end + 1;
      ++next_.line_;

      while (true) {
        auto pos = end + 1;
        end = std::find(pos, data_.end(), '\n');
        if (end == data_.end()) {
          next_.cursor_ = data_.end();
          next_.line_ = -1;
          return std::unexpected<data::tparse_error>{
              std::in_place, current_.line_,
              std::string_view{current_.cursor_, data_.end()},
              "end of file before multiline terminator was found"};
        }

        if (std::string_view{pos, end} == ">>>") {
          value = std::string_view{begin, pos - 1};
          break;
        }

        ++next_.line_;
      }
      // The last line is not "counted" since it always counts one line.
    }

    ++next_.line_;
    next_.cursor_ = end + (end != data_.end());

    return tresult{tresult::pair,
                   {std::string_view{current_.cursor_, separator}, value}};
  }

  std::string_view data_{};

  /** The state of the parser. */
  struct state {
    /** The position to start processing. */
    std::string_view::iterator cursor_;
    /** The line being processed. */
    int line_{1};
  };

  /**
   * Set to the state at the start of parse.
   *
   * This means current_.cursor_ is the start of parsing, so the start of the
   * line(s) begin parsed.
   */
  state current_;
  /**
   * Updated during parsing.
   *
   * The state at the end is the start state for the next parse cycle.
   * When an error occurred the line is set ot -1.
   */
  state next_;
};

enum class tfield_type { id, string, color, boolean, status, date, id_list };

enum class tfield_requirement { mandatory, optional };

struct tid {
  enum class ttarget { label, project, group, task };
  std::optional<std::size_t> value;
  ttarget target;
  bool self;
};

struct tstring {
  std::optional<std::string> value;
};

struct tcolor {
  std::optional<data::tcolor> value;
};

struct tboolean {
  std::optional<bool> value;
};

struct tstatus {
  std::optional<data::ttask::tstatus> value;
};

struct tdate {
  std::optional<std::chrono::year_month_day> value;
};

struct tid_list {
  enum class ttarget { label, project, group, task };
  std::optional<std::vector<std::size_t>> value;
  ttarget target;
  bool self;
};

struct tfield {
  std::string_view name;
  tfield_type type;
  tfield_requirement requirement;
  std::variant<tid, tstring, tcolor, tboolean, tstatus, tdate, tid_list> value;
};

std::optional<data::tparse_error>
parse_id(tfield &field, std::string_view input, int line_no) {
  auto &id = std::get<tid>(field.value);
  if (id.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("duplicate entry for field »{}«", field.name)};

  std::size_t value;
  std::from_chars_result status =
      std::from_chars(input.data(), input.data() + input.size(), value);
  if (status.ec != std::errc{})
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("invalid number for field »{}«", field.name)};

  if (status.ptr != input.data() + input.size())
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("number contains non-digits for field »{}«", field.name)};

  if (field.requirement == tfield_requirement::mandatory && value == 0)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("zero is not a valid value for mandatory id field »{}«",
                    field.name)};

  id.value = value;
  return {};
}

std::optional<data::tparse_error>
parse_string(tfield &field, std::string_view input, int line_no) {
  auto &string = std::get<tstring>(field.value);
  if (string.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("duplicate entry for field »{}«", field.name)};

  if (field.requirement == tfield_requirement::mandatory && input.empty())
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("an empty string is not a valid value for mandatory string "
                    "field »{}«",
                    field.name)};
  string.value = input;
  return {};
}

std::optional<data::tparse_error>
parse_color(tfield &field, std::string_view input, int line_no) {
  auto &color = std::get<tcolor>(field.value);
  if (color.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("duplicate entry for field »{}«", field.name)};

  if (field.requirement == tfield_requirement::mandatory && input.empty())
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("an empty string is not a valid value for mandatory color "
                    "field »{}«",
                    field.name)};

  constexpr std::array<std::string_view, 16> color_names{
      "black", "RED", "GREEN", "YELLOW", "BLUE", "MAGENTA", "CYAN", "gray",
      "GRAY",  "red", "green", "yellow", "blue", "magenta", "cyan", "white"};

  auto iter = std::ranges::find(color_names, input);
  if (iter == color_names.end())
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("invalid color value for field »{}«", field.name)};

  color.value = static_cast<data::tcolor>(iter - color_names.begin());
  return {};
}

std::optional<data::tparse_error>
parse_boolean(tfield &field, std::string_view input, int line_no) {
  auto &boolean = std::get<tboolean>(field.value);
  if (boolean.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("duplicate entry for field »{}«", field.name)};

  if (input == "false")
    boolean.value = false;
  else if (input == "true")
    boolean.value = true;
  else
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("invalid boolean value for field »{}«", field.name)};

  return {};
}

std::optional<data::tparse_error>
parse_status(tfield &field, std::string_view input, int line_no) {
  auto &status = std::get<tstatus>(field.value);
  if (status.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("duplicate entry for field »{}«", field.name)};

  if (input == "backlog")
    status.value = data::ttask::tstatus::backlog;
  else if (input == "selected")
    status.value = data::ttask::tstatus::selected;
  else if (input == "progress")
    status.value = data::ttask::tstatus::progress;
  else if (input == "review")
    status.value = data::ttask::tstatus::review;
  else if (input == "done")
    status.value = data::ttask::tstatus::done;
  else if (input == "discarded")
    status.value = data::ttask::tstatus::discarded;
  else
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("invalid status value for field »{}«", field.name)};

  return {};
}

std::optional<data::tparse_error>
parse_date(tfield &field, std::string_view input, int line_no) {
  auto &date = std::get<tdate>(field.value);
  if (date.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("duplicate entry for field »{}«", field.name)};

  std::size_t end = input.find('.');
  if (end == std::string::npos)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("month separator not found for field »{}«", field.name)};

  int year;
  std::from_chars_result status =
      std::from_chars(input.data(), input.data() + end, year);
  if (status.ec != std::errc{})
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("invalid year for field »{}«", field.name)};
  if (status.ptr != input.data() + end)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("year contains non-digits for field »{}«", field.name)};

  std::string_view data = input.substr(end + 1);
  end = data.find('.');
  if (end == std::string::npos)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("day separator not found for field »{}«", field.name)};

  unsigned month;
  status = std::from_chars(data.data(), data.data() + end, month);
  if (status.ec != std::errc{})
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("invalid month for field »{}«", field.name)};
  if (status.ptr != data.data() + end)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("month contains non-digits for field »{}«", field.name)};

  data = data.substr(end + 1);

  unsigned day;
  status = std::from_chars(data.data(), data.data() + data.size(), day);
  if (status.ec != std::errc{})
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("invalid day for field »{}«", field.name)};
  if (status.ptr != data.data() + data.size())
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("day contains non-digits for field »{}«", field.name)};

  std::chrono::year_month_day result{std::chrono::year{year},
                                     std::chrono::month{month},
                                     std::chrono::day{day}};

  if (!result.ok())
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("not a valid date for field »{}«", field.name)};

  date.value = result;
  return {};
}

std::optional<data::tparse_error>
parse_id_list(tfield &field, std::string_view input, int line_no) {
  auto &id_list = std::get<tid_list>(field.value);
  if (id_list.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, input,
        std::format("duplicate entry for field »{}«", field.name)};

  std::vector<std::size_t> result;
  std::string_view data = input;
  while (!data.empty()) {
    if (data[0] == ' ') {
      data.remove_prefix(1);
      continue;
    }

    const char *end = std::find(data.data(), data.data() + data.size(), ',');

    std::size_t value;
    std::from_chars_result status = std::from_chars(data.data(), end, value);
    if (status.ec != std::errc{})
      return std::optional<data::tparse_error>{
          std::in_place, line_no, input,
          std::format("invalid number for field »{}«", field.name)};

    if (status.ptr != end)
      return std::optional<data::tparse_error>{
          std::in_place, line_no, input,
          std::format("number contains non-digits for field »{}«", field.name)};

    if (value == 0)
      return std::optional<data::tparse_error>{
          std::in_place, line_no, input,
          std::format("zero is not a valid value for an id list field »{}«",
                      field.name)};

    result.push_back(value);
    if (end == data.data() + data.size())
      break;

    data = std::string_view{end + 1, data.data() + data.size()};
  }

  id_list.value = std::move(result);
  return {};
}

std::optional<data::tparse_error>
parse_element(tfield &field, std::string_view input, int line_no) {
  switch (field.type) {
  case tfield_type::id:
    return parse_id(field, input, line_no);
  case tfield_type::string:
    return parse_string(field, input, line_no);
  case tfield_type::color:
    return parse_color(field, input, line_no);
  case tfield_type::boolean:
    return parse_boolean(field, input, line_no);
  case tfield_type::status:
    return parse_status(field, input, line_no);
  case tfield_type::date:
    return parse_date(field, input, line_no);
  case tfield_type::id_list:
    return parse_id_list(field, input, line_no);
  }
}

/// *** Validate ***

template <class Container, class Projection, class T>
std::optional<data::tparse_error>
validate_unique(const Container &container, Projection projection,
                const T &value, const data::tstate &state,
                std::string_view field, int line_no)

{
#if 0
  // Contains hasn't been implemented yet.
  if (!std::ranges::contains(container, value, projection))
    return {};
#else
  if (std::ranges::find(container, value, projection) ==
      std::ranges::end(container))
    return {};
#endif

  return std::optional<data::tparse_error>{
      std::in_place, line_no, "",
      std::format("id field »{}« has multiple values »{}«", field, value)};
}

template <class Container, class Projection, class T>
std::optional<data::tparse_error>
validate_exists(const Container &container, Projection projection,
                const T &value, const data::tstate &state,
                std::string_view field, int line_no) {
#if 0
  // Contains hasn't been implemented yet.
  if (std::ranges::contains(container, value, projection))
    return {};
#else
  if (std::ranges::find(container, value, projection) !=
      std::ranges::end(container))
    return {};
#endif

  return std::optional<data::tparse_error>{
      std::in_place, line_no, "",
      std::format("id field »{}« has no linked record for value »{}«", field,
                  value)};
}

std::optional<data::tparse_error>
validate_id(const tfield &field, const data::tstate &state, int line_no) {

  const auto &id = std::get<tid>(field.value);
  if (!id.value || *id.value == 0)
    return field.requirement == tfield_requirement::optional
               ? std::optional<data::tparse_error>{}
               : std::optional<data::tparse_error>{
                     std::in_place, line_no, "",
                     std::format("missing mandatory field »{}«", field.name)};

  std::optional<data::tparse_error> result;
  switch (id.target) {
  case tid::ttarget::label:
    return id.self ? validate_unique(state.labels, &data::tlabel::id, *id.value,
                                     state, field.name, line_no)
                   : validate_exists(state.labels, &data::tlabel::id, *id.value,
                                     state, field.name, line_no);

  case tid::ttarget::project:
    return id.self ? validate_unique(state.projects, &data::tproject::id,
                                     *id.value, state, field.name, line_no)
                   : validate_exists(state.projects, &data::tproject::id,
                                     *id.value, state, field.name, line_no);

  case tid::ttarget::group:
    return id.self ? validate_unique(state.groups, &data::tgroup::id, *id.value,
                                     state, field.name, line_no)
                   : validate_exists(state.groups, &data::tgroup::id, *id.value,
                                     state, field.name, line_no);

  case tid::ttarget::task:
    return id.self ? validate_unique(state.tasks, &data::ttask::id, *id.value,
                                     state, field.name, line_no)
                   : validate_exists(state.tasks, &data::ttask::id, *id.value,
                                     state, field.name, line_no);
  }
}

std::optional<data::tparse_error>
validate_string(const tfield &field, const data::tstate &state, int line_no) {
  const auto &string = std::get<tstring>(field.value);
  if (field.requirement == tfield_requirement::mandatory && !string.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, "",
        std::format("missing mandatory field »{}«", field.name)};

  return {};
}

std::optional<data::tparse_error>
validate_color(const tfield &field, const data::tstate &state, int line_no) {
  const auto &color = std::get<tcolor>(field.value);
  if (field.requirement == tfield_requirement::mandatory && !color.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, "",
        std::format("missing mandatory field »{}«", field.name)};

  return {};
}

std::optional<data::tparse_error>
validate_boolean(const tfield &field, const data::tstate &state, int line_no) {
  const auto &boolean = std::get<tboolean>(field.value);
  if (field.requirement == tfield_requirement::mandatory && !boolean.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, "",
        std::format("missing mandatory field »{}«", field.name)};

  return {};
}

std::optional<data::tparse_error>
validate_status(const tfield &field, const data::tstate &state, int line_no) {
  const auto &status = std::get<tstatus>(field.value);
  if (field.requirement == tfield_requirement::mandatory && !status.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, "",
        std::format("missing mandatory field »{}«", field.name)};

  return {};
}

std::optional<data::tparse_error>
validate_date(const tfield &field, const data::tstate &state, int line_no) {
  const auto &date = std::get<tdate>(field.value);
  if (field.requirement == tfield_requirement::mandatory && !date.value)
    return std::optional<data::tparse_error>{
        std::in_place, line_no, "",
        std::format("missing mandatory field »{}«", field.name)};

  return {};
}

std::optional<data::tparse_error>
validate_id_list(const tfield &field, const data::tstate &state, int line_no) {
  const auto &id_list = std::get<tid_list>(field.value);

  if (!id_list.value || id_list.value->empty()) {
    if (field.requirement == tfield_requirement::mandatory)
      return std::optional<data::tparse_error>{
          std::in_place, line_no, "",
          std::format("missing mandatory field »{}«", field.name)};
    else
      return {};
  }

  if (id_list.self)
    return {};

  switch (id_list.target) {
  case tid_list::ttarget::label:
    for (const auto value : *id_list.value) {
      std::optional<data::tparse_error> error = validate_exists(
          state.labels, &data::tlabel::id, value, state, field.name, line_no);
      if (error)
        return error;
    }
    break;

  case tid_list::ttarget::project:
    for (const auto value : *id_list.value) {
      std::optional<data::tparse_error> error =
          validate_exists(state.projects, &data::tproject::id, value, state,
                          field.name, line_no);
      if (error)
        return error;
    }
    break;

  case tid_list::ttarget::group:
    for (const auto value : *id_list.value) {
      std::optional<data::tparse_error> error = validate_exists(
          state.groups, &data::tgroup::id, value, state, field.name, line_no);
      if (error)
        return error;
    }
    break;

  case tid_list::ttarget::task:
    for (const auto value : *id_list.value) {
      std::optional<data::tparse_error> error = validate_exists(
          state.tasks, &data::ttask::id, value, state, field.name, line_no);
      if (error)
        return error;
    }
    break;
  }

  return {};
}

std::optional<data::tparse_error>
validate_field(const tfield &field, const data::tstate &state, int line_no) {

  switch (field.type) {
  case tfield_type::id:
    return validate_id(field, state, line_no);
  case tfield_type::string:
    return validate_string(field, state, line_no);
  case tfield_type::color:
    return validate_color(field, state, line_no);
  case tfield_type::boolean:
    return validate_boolean(field, state, line_no);
  case tfield_type::status:
    return validate_status(field, state, line_no);
  case tfield_type::date:
    return validate_date(field, state, line_no);
  case tfield_type::id_list:
    return validate_id_list(field, state, line_no);
  }
}
/// *** PARSE

std::optional<data::tparse_error>
parse_record(data::tstate &state, parser &parser, std::span<tfield> fields) {

  // *** PARSE ***
  int line = parser.line();
  bool done = false;
  do {

    std::expected<parser::tresult, data::tparse_error> line = parser.parse();

    // TODO PARSE ERROR should be same as parser::terror
    if (!line)
      return line.error();

    switch (line->type) {

    case parser::tresult::eof:
    case parser::tresult::empty:
      done = true;
      break;

    case parser::tresult::header:
      return std::optional<data::tparse_error>{std::in_place, parser.line(),
                                               line->data[0],
                                               "headers can't be nested"};

    case parser::tresult::pair: {
      std::span<tfield>::iterator iter =
          std::ranges::find(fields, line->data[0], &tfield::name);

      if (iter == fields.end())
        return std::optional<data::tparse_error>{
            std::in_place, parser.line(),
            std::string_view{line->data[0].begin(), line->data[1].end()},
            "invalid field name"};

      std::optional<data::tparse_error> error =
          parse_element(*iter, line->data[1], parser.line());

      if (error)
        return error;
    } break;
    }
  } while (!done);

  // *** VALIDATE ***

  for (const auto &field : fields) {
    std::optional<data::tparse_error> error =
        validate_field(field, state, line);
    if (error)
      return error;
  }

  return {};
}

std::optional<data::tparse_error> parse_label(data::tstate &state,
                                              parser &parser) {
  std::array record{
      tfield{"id", tfield_type::id, tfield_requirement::mandatory,
             tid{.target = tid::ttarget::label, .self = true}},
      tfield{"name", tfield_type::string, tfield_requirement::mandatory,
             tstring{}},
      tfield{"description", tfield_type::string, tfield_requirement::optional,
             tstring{}},
      tfield{"color", tfield_type::color, tfield_requirement::optional,
             tcolor{}},
  };

  std::optional<data::tparse_error> error = parse_record(state, parser, record);
  if (error)
    return *error;

  state.labels.emplace_back(
      std::get<tid>(record[0].value).value.value(),
      std::get<tstring>(record[1].value).value.value(),
      std::get<tstring>(record[2].value).value.value_or(""),
      std::get<tcolor>(record[3].value).value.value_or(data::tcolor::black));

  return {};
}

std::optional<data::tparse_error> parse_project(data::tstate &state,
                                                parser &parser) {
  std::array record{
      tfield{"id", tfield_type::id, tfield_requirement::mandatory,
             tid{.target = tid::ttarget::project, .self = true}},
      tfield{"name", tfield_type::string, tfield_requirement::mandatory,
             tstring{}},
      tfield{"description", tfield_type::string, tfield_requirement::optional,
             tstring{}},
      tfield{"color", tfield_type::color, tfield_requirement::optional,
             tcolor{}},
      tfield{"active", tfield_type::boolean, tfield_requirement::optional,
             tboolean{}},
  };

  std::optional<data::tparse_error> error = parse_record(state, parser, record);
  if (error)
    return *error;

  state.projects.emplace_back(
      std::get<tid>(record[0].value).value.value(),
      std::get<tstring>(record[1].value).value.value(),
      std::get<tstring>(record[2].value).value.value_or(""),
      std::get<tcolor>(record[3].value).value.value_or(data::tcolor::black),
      std::get<tboolean>(record[4].value).value.value_or(true));

  return {};
}

std::optional<data::tparse_error> parse_group(data::tstate &state,
                                              parser &parser) {
  std::array record{
      tfield{"id", tfield_type::id, tfield_requirement::mandatory,
             tid{.target = tid::ttarget::group, .self = true}},
      tfield{"project", tfield_type::id, tfield_requirement::mandatory,
             tid{.target = tid::ttarget::project, .self = false}},
      tfield{"name", tfield_type::string, tfield_requirement::mandatory,
             tstring{}},
      tfield{"description", tfield_type::string, tfield_requirement::optional,
             tstring{}},
      tfield{"color", tfield_type::color, tfield_requirement::optional,
             tcolor{}},
      tfield{"active", tfield_type::boolean, tfield_requirement::optional,
             tboolean{}},
  };

  std::optional<data::tparse_error> error = parse_record(state, parser, record);
  if (error)
    return *error;

  state.groups.emplace_back(
      std::get<tid>(record[0].value).value.value(),
      std::get<tid>(record[1].value).value.value(),
      std::get<tstring>(record[2].value).value.value(),
      std::get<tstring>(record[3].value).value.value_or(""),
      std::get<tcolor>(record[4].value).value.value_or(data::tcolor::black),
      std::get<tboolean>(record[5].value).value.value_or(true));

  return {};
}

std::optional<data::tparse_error> parse_task(data::tstate &state,
                                             parser &parser) {
  int line = parser.line();

  std::array record{
      tfield{"id", tfield_type::id, tfield_requirement::mandatory,
             tid{.target = tid::ttarget::task, .self = true}},
      tfield{"project", tfield_type::id, tfield_requirement::optional,
             tid{.target = tid::ttarget::project, .self = false}},
      tfield{"group", tfield_type::id, tfield_requirement::optional,
             tid{.target = tid::ttarget::group, .self = false}},
      tfield{"title", tfield_type::string, tfield_requirement::mandatory,
             tstring{}},
      tfield{"description", tfield_type::string, tfield_requirement::optional,
             tstring{}},
      tfield{"status", tfield_type::status, tfield_requirement::optional,
             tstatus{}},
      tfield{"after", tfield_type::date, tfield_requirement::optional, tdate{}},
      tfield{"labels", tfield_type::id_list, tfield_requirement::optional,
             tid_list{.target = tid_list::ttarget::label, .self = false}},
      tfield{"dependencies", tfield_type::id_list, tfield_requirement::optional,
             tid_list{.target = tid_list::ttarget::task, .self = true}},
      tfield{"requirements", tfield_type::id_list, tfield_requirement::optional,
             tid_list{.target = tid_list::ttarget::group, .self = false}},
  };

  std::optional<data::tparse_error> error = parse_record(state, parser, record);
  if (error)
    return *error;

  // Project and group are mutually exclusive fields.
  std::size_t id = std::get<tid>(record[0].value).value.value();
  std::size_t project = std::get<tid>(record[1].value).value.value_or(0);
  std::size_t group = std::get<tid>(record[2].value).value.value_or(0);
  if (project && group)
    return std::optional<data::tparse_error>{
        std::in_place, line, "",
        std::format("task »{}« has both a »group« and a »project« set", id)};

  state.tasks.emplace_back(
      id,      //
      project, //
      group,   //
      std::get<tstring>(record[3].value).value.value(),
      std::get<tstring>(record[4].value).value.value_or(""),
      std::get<tstatus>(record[5].value)
          .value.value_or(data::ttask::tstatus::backlog),
      std::get<tdate>(record[6].value).value, // the target type is an optional
      std::get<tid_list>(record[7].value)
          .value.value_or(std::vector<std::size_t>{}),
      std::get<tid_list>(record[8].value)
          .value.value_or(std::vector<std::size_t>{}),
      std::get<tid_list>(record[9].value)
          .value.value_or(std::vector<std::size_t>{}));

  return {};
}

std::optional<data::tparse_error>
parse_header(data::tstate &state, parser &parser, std ::string_view header) {
  if (header == "[label]")
    return parse_label(state, parser);
  if (header == "[project]")
    return parse_project(state, parser);
  if (header == "[group]")
    return parse_group(state, parser);
  if (header == "[task]")
    return parse_task(state, parser);

  return data::tparse_error{parser.line(), header, "found unknown header"};
}

namespace hack {

using namespace data;

// This is the real function, but passing this type from the module causes all
// kinds of errors. Instead use a dummy wrapper for now. Obviously this is a
// hack that may cause memory leaks.
[[nodiscard]] std::expected<std::unique_ptr<tstate>, tparse_error>
parse(std::string_view input) {
  auto state = std::make_unique<tstate>();

  parser parser(input);
  while (true) {

    std::expected<parser::tresult, tparse_error> line = parser.parse();

    if (!line)
      return std::unexpected{line.error()};

    switch (line->type) {
    case parser::tresult::eof:
      return std::move(state);

    case parser::tresult::empty:
      /* DO NOTHING */
      break;

    case parser::tresult::header: {
      std::optional<tparse_error> error =
          parse_header(*state, parser, line->data[0]);
      if (error)
        return std::unexpected{*error};

    } break;

    case parser::tresult::pair:
      return std::unexpected<tparse_error>{
          std::in_place, parser.line(),
          std::string_view{line->data[0].begin(), line->data[1].end()},
          "value is not attached to a header"};
    }
  }

  return std::move(state);
}

} // namespace hack

export namespace data {

/**
 * Parses the input data.
 *
 * Returns the parsed contents or the error.
 *
 * @note The lifetime of the input should outlast the returned error. The
 * returned state has no lifetime dependency on the input.
 *
 * @note The returned values are raw pointers due to a bug in Clang. When using
 * modules it can't properly find the properly constrained destructor of
 * std::excepted. This means the type hast to be trivial.
 * Once this is fixed the state will be returned by a unqiue_ptr and the error
 * by value.
 */
[[nodiscard]] std::expected<tstate *, tparse_error *>
parse(std::string_view input) {
  std::expected<std::unique_ptr<tstate>, tparse_error> result =
      hack::parse(input);

  if (result)
    return result->release();

  return std::unexpected<tparse_error *>(new tparse_error(result.error()));
}

} // namespace data
