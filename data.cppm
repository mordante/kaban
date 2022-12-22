module;
#include <charconv>
#include <chrono>
#include <fstream>
#include <string_view>
#include <vector>
export module data;

// TODO DO NOT EXPORT
export std::size_t parse_id(std::string_view input) {
  std::size_t result;
  std::from_chars_result status =
      std::from_chars(input.data(), input.data() + input.size(), result);
  if (status.ec != std::errc{})
    throw 42;

  if (status.ptr != input.data() + input.size())
    throw 42;

  return result;
}

// TODO DO NOT EXPORT
export std::vector<std::size_t> parse_id_list(std::string_view input) {
  std::vector<std::size_t> result;
  if (input.empty())
    return result;
  const char *begin = input.data();
  const char *end = input.data() + input.size();

  while (true) {
    std::size_t value;
    std::from_chars_result status = std::from_chars(begin, end, value);
    if (status.ec != std::errc{})
      throw 42;

    result.push_back(value);
    begin = status.ptr;
    if (begin == end)
      return result;

    if (*begin++ != ',' || *begin++ != ' ')
      throw 42;
  }
}

// TODO DO NOT EXPORT
export bool parse_bool(std::string_view input) {
  if (input == "true")
    return true;

  if (input == "false")
    return false;

  throw 42;
}

// TODO DO NOT EXPORT
export std::chrono::year_month_day parse_date(std::string_view input) {
  std::size_t end = input.find('.');
  if (end == std::string::npos)
    throw 42;

  std::chrono::year year{static_cast<int>(parse_id(input.substr(0, end)))};
  input = input.substr(end + 1);

  end = input.find('.');
  if (end == std::string::npos)
    throw 42;

  std::chrono::month month{
      static_cast<unsigned>(parse_id(input.substr(0, end)))};
  input = input.substr(end + 1);

  std::chrono::day day{static_cast<unsigned>(parse_id(input))};

  return {year, month, day};
}

template <class F> struct delay {
  [[nodiscard]] explicit delay(F &&f) : f_(std::move(f)) {}

  ~delay() { f_(); }

  F f_;
};

// TODO DO SOMETHING
void validate_color(std::string_view input) {}

// TODO DO NOT EXPORT
export struct parser {
  explicit parser(std::istream &stream)
      : data_(std::istreambuf_iterator<char>(stream), {}) {}
  enum tresult { eof, empty, header, pair };

  std::pair<tresult, std::array<std::string_view, 2>> parse() {

    if (cursor_ == data_.end())
      return {tresult::eof, {}};

    switch (*cursor_) {
    case '\n':
      ++cursor_;
      return {tresult::empty, {}};

    case '[': {
      auto end = std::find(cursor_ + 1, data_.end(), '\n');
      delay _{[&] { cursor_ = end + (end != data_.end()); }};
      return {tresult::header, {std::string_view{cursor_, end}, {}}};
    }
    }

    auto separator = std::find(cursor_ + 1, data_.end(), '=');
    if (separator == data_.end())
      throw 42;

    auto begin = separator + 1;
    auto end = std::find(begin, data_.end(), '\n');

    std::string_view value{begin, end};
    if (value == "<<<") {
      // multiline value
      if (end == data_.end())
        throw 42;
      begin = end + 1;

      while (true) {
        auto pos = end + 1;
        end = std::find(pos, data_.end(), '\n');
        if (end == data_.end())
          throw 42;
        if (std::string_view{pos, end} == ">>>") {
          value = std::string_view{begin, pos - 1};
          break;
        }
      }
    }

    delay _{[&] { cursor_ = end + (end != data_.end()); }};

    return {tresult::pair, {std::string_view{cursor_, separator}, value}};
  }

private:
  std::string data_{};
  std::string::iterator cursor_{data_.begin()};
};

export struct label {
  explicit label(parser &parser) {
    parse(parser);
    validate();
  }
  std::size_t id;
  std::string name;
  std::string description;
  std::string color;

private:
  void parse(parser &parser) {

    while (true) {
      std::pair<parser::tresult, std::array<std::string_view, 2>> line =
          parser.parse();
      switch (line.first) {
      case parser::tresult::eof:
      case parser::tresult::empty:
        return;

      case parser::tresult::header:
        throw 42;
      case parser::tresult::pair:

        if (line.second[0] == "id")
          id = parse_id(line.second[1]);
        else if (line.second[0] == "name")
          name = line.second[1];
        else if (line.second[0] == "description")
          description = line.second[1];
        else if (line.second[0] == "color")
          color = line.second[1];
        else
          throw 42;
      }
    }
  }

  void validate() const {
    if (id == 0)
      throw 42;
    if (name.empty())
      throw 42;
  }
};
export struct project {
  explicit project(parser &parser) {
    parse(parser);
    validate();
  }
  std::size_t id;
  std::string name;
  std::string description;
  std::string color;
  bool active{true};

private:
  void parse(parser &parser) {

    while (true) {
      std::pair<parser::tresult, std::array<std::string_view, 2>> line =
          parser.parse();
      switch (line.first) {
      case parser::tresult::eof:
      case parser::tresult::empty:
        return;

      case parser::tresult::header:
        throw 42;
      case parser::tresult::pair:

        if (line.second[0] == "id")
          id = parse_id(line.second[1]);
        else if (line.second[0] == "name")
          name = line.second[1];
        else if (line.second[0] == "description")
          description = line.second[1];
        else if (line.second[0] == "color")
          color = line.second[1];
        else if (line.second[0] == "active")
          active = parse_bool(line.second[1]);
        else
          throw 42;
      }
    }
  }

  void validate() const {
    if (id == 0)
      throw 42;
    if (name.empty())
      throw 42;
    if (!color.empty())
      validate_color(color);
  }
};

export struct group {
  explicit group(parser &parser) {
    parse(parser);
    validate();
  }
  std::size_t id;
  std::string name;
  std::string description;
  size_t project{0};
  std::string color;
  bool active{true};

private:
  void parse(parser &parser) {

    while (true) {
      std::pair<parser::tresult, std::array<std::string_view, 2>> line =
          parser.parse();
      switch (line.first) {
      case parser::tresult::eof:
      case parser::tresult::empty:
        return;

      case parser::tresult::header:
        throw 42;
      case parser::tresult::pair:

        if (line.second[0] == "id")
          id = parse_id(line.second[1]);
        else if (line.second[0] == "project")
          project = parse_id(line.second[1]);
        else if (line.second[0] == "name")
          name = line.second[1];
        else if (line.second[0] == "description")
          description = line.second[1];
        else if (line.second[0] == "active")
          active = parse_bool(line.second[1]);
        else if (line.second[0] == "color")
          color = line.second[1];
        else
          throw 42;
      }
    }
  }

  void validate() const {
    if (id == 0)
      throw 42;
    if (project == 0)
      throw 42;
    if (name.empty())
      throw 42;
    if (!color.empty())
      validate_color(color);
  }
};

export struct task {

  explicit task(parser &parser) {
    parse(parser);
    validate();
  }

  enum class tstatus {
    backlog,
    selected, /**< Selected for development. */
    progress, /**< In progress. */
    review,   /**< In review. */
    done,     /**< Completed the work. */
    discarded /**< The task has been discarded. */
  };

  std::size_t id;
  std::string title;
  std::string description;
  tstatus status{tstatus::backlog};
  size_t project{0};
  size_t group{0};
  std::vector<std::size_t> labels;
  std::vector<std::size_t> dependencies;
  std::vector<std::size_t> requirements;
  std::optional<std::chrono::year_month_day> after;

private:
  void parse(parser &parser) {

    while (true) {
      std::pair<parser::tresult, std::array<std::string_view, 2>> line =
          parser.parse();
      switch (line.first) {
      case parser::tresult::eof:
      case parser::tresult::empty:
        return;

      case parser::tresult::header:
        throw 42;

      case parser::tresult::pair:
        if (line.second[0] == "id")
          id = parse_id(line.second[1]);
        else if (line.second[0] == "project")
          project = parse_id(line.second[1]);
        else if (line.second[0] == "group")
          group = parse_id(line.second[1]);
        else if (line.second[0] == "title")
          title = line.second[1];
        else if (line.second[0] == "description")
          description = line.second[1];
        else if (line.second[0] == "status")
          status = parse_status(line.second[1]);
        else if (line.second[0] == "labels")
          labels = parse_id_list(line.second[1]);
        else if (line.second[0] == "dependencies")
          dependencies = parse_id_list(line.second[1]);
        else if (line.second[0] == "requirements")
          requirements = parse_id_list(line.second[1]);
        else if (line.second[0] == "after")
          after = parse_date(line.second[1]);
        else
          throw 42;
      }
    }
  }

  void validate() const {
    if (id == 0)
      throw 42;
    if (title.empty())
      throw 42;
    if (project && group)
      throw 42;
  }
  tstatus parse_status(std::string_view input) {
    if (input == "backlog")
      return task::tstatus::backlog;
    if (input == "selected")
      return task::tstatus::selected;
    if (input == "progress")
      return task::tstatus::progress;
    if (input == "review")
      return task::tstatus::review;
    if (input == "done")
      return task::tstatus::done;
    if (input == "discarded")
      return task::tstatus::discarded;

    throw 42;
  }
};

// TODO DO NOT EXPORT
export std::vector<project> projects;
// TODO DO NOT EXPORT
export std::vector<group> groups;
// TODO DO NOT EXPORT
export std::vector<label> labels;
// TODO DO NOT EXPORT
export std::vector<task> tasks;

// TODO DO NOT EXPORT
export void parse(std::ifstream &file) {
  parser parser(file);
  while (true) {
    std::pair<parser::tresult, std::array<std::string_view, 2>> line =
        parser.parse();
    switch (line.first) {
    case parser::tresult::eof:
      return;
    case parser::tresult::empty:
      /* DO NOTHING */
      break;

    case parser::tresult::header:
      if (line.second[0] == "[project]")
        projects.emplace_back(parser);
      else if (line.second[0] == "[label]")
        labels.emplace_back(parser);
      else if (line.second[0] == "[group]")
        groups.emplace_back(parser);
      else if (line.second[0] == "[task]")
        tasks.emplace_back(parser);
      else
        throw 42;
      break;

    case parser::tresult::pair:
      throw 42;
    }
  }
}
