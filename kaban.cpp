#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <sstream> // multiline

std::size_t parse_id(std::string_view input) {
  std::size_t result;
  std::from_chars_result status =
      std::from_chars(input.data(), input.data() + input.size(), result);
  if (status.ec != std::errc{})
    throw 42;

  if (status.ptr != input.data() + input.size())
    throw 42;

  return result;
}

std::vector<std::size_t> parse_id_list(std::string_view input) {
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

bool parse_bool(std::string_view input) {
  if (input == "true")
    return true;

  if (input == "false")
    return false;

  throw 42;
}

std::chrono::year_month_day parse_date(std::string_view input) {
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

struct parser {
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

struct project {
  explicit project(parser &parser) {
    parse(parser);
    validate();
  }
  std::size_t id;
  std::string name;
  std::string description;
  bool active{true};

private:
  void parse(parser &parser);

  void validate() const {
    if (id == 0)
      throw 42;
    if (name.empty())
      throw 42;
  }
};

struct group {
  explicit group(parser &parser) {
    parse(parser);
    validate();
  }
  std::size_t id;
  std::string name;
  std::string description;
  size_t project{0};
  bool active{true};

private:
  void parse(parser &parser);

  void validate() const {
    if (id == 0)
      throw 42;
    if (project == 0)
      throw 42;
    if (name.empty())
      throw 42;
  }
};

struct label {
  explicit label(parser &parser) {
    parse(parser);
    validate();
  }
  std::size_t id;
  std::string name;
  std::string description;

private:
  void parse(parser &parser);

  void validate() const {
    if (id == 0)
      throw 42;
    if (name.empty())
      throw 42;
  }
};

struct task {

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
  std::vector<std::size_t> dependencies;
  std::vector<std::size_t> requirements;
  std::optional<std::chrono::year_month_day> after;

private:
  void parse(parser &parser);

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

std::vector<project> projects;
std::vector<group> groups;
std::vector<label> labels;
std::vector<task> tasks;

void project::parse(parser &parser) {

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
      else if (line.second[0] == "active")
        active = parse_bool(line.second[1]);
      else
        throw 42;
    }
  }
}

void group::parse(parser &parser) {

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
      else
        throw 42;
    }
  }
}

void label::parse(parser &parser) {

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
      else
        throw 42;
    }
  }
}

void task::parse(parser &parser) {

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

void parse(std::ifstream &file) {
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

bool is_complete(task::tstatus status) {
  return status == task::tstatus::done || status == task::tstatus::discarded;
}

bool is_blocked(const task &task) {
  for (auto id : task.dependencies) {
    auto it = std::find_if(tasks.begin(), tasks.end(),
                           [id](const auto &task) { return task.id == id; });
    if (it == tasks.end())
      throw 42;

    if (!is_complete(it->status))
      return true;
  }
  // TODO Validate requirements
  if (task.after)
    return static_cast<std::chrono::sys_days>(*task.after) <=
           std::chrono::system_clock::now();
  return false;
}

bool is_active(const task &task) {
  if (task.project == 0)
    return true;

  auto it = std::find_if(
      projects.begin(), projects.end(),
      [id = task.project](const auto &project) { return project.id == id; });
  if (it == projects.end())
    throw 42;

  return it->active;
}

std::string_view get_title(std::size_t id) {
  auto it = std::find_if(tasks.begin(), tasks.end(),
                         [id](const auto &task) { return task.id == id; });

  if (it == tasks.end())
    throw 42;

  return it->title;
}

std::string_view get_project_name(std::size_t id) {
  auto it =
      std::find_if(projects.begin(), projects.end(),
                   [id](const auto &project) { return project.id == id; });

  if (it == projects.end())
    throw 42;

  return it->name;
}

std::string_view get_group_name(std::size_t id) {
  auto it = std::find_if(groups.begin(), groups.end(),
                         [id](const auto &group) { return group.id == id; });

  if (it == groups.end())
    throw 42;

  return it->name;
}

void print(std::string_view title, const std::vector<task> &tasks) {
  if (tasks.empty())
    return;

  std::cout << std::format("[{}]\n", title);
  for (const auto &task : tasks) {
    if (task.project)
      std::cout << std::format("- {:3} [{}] {}\n", task.id,
                               get_project_name(task.project), task.title);
    else
      std::cout << std::format("- {:3} {}\n", task.id, task.title);

    if (!task.description.empty())
      std::cout << std::format("      - {}\n", task.description);

    if (!task.dependencies.empty())
      for (auto id : task.dependencies)
        std::cout << std::format("      x {:3} {}\n", id, get_title(id));
  }
  std::cout << '\n';
}

namespace ftxui {

Element multiline_text(const std::string &the_text) {
  Elements output;
  std::stringstream ss(the_text);
  std::string line;
  while (std::getline(ss, line)) {
    output.push_back(paragraph(line));
  }
  return vbox(output);
}

} // namespace ftxui

namespace gui {

struct ticket {
  ticket(const task &task) : task_(std::addressof(task)) {

    ftxui::Components result;

    result.push_back(ftxui::Renderer([&] {
      return ftxui::text(task_->project
                             ? std::format("{:3} [{}] {}", task_->id,
                                           get_project_name(task_->project),
                                           task_->title)
                             : std::format("{:3} {}", task_->id, task_->title));
    }));

    if (!task_->description.empty()) {
      show_description = task_->status == task::tstatus::progress;
      result.push_back(ftxui::Container::Horizontal(
          {ftxui::Checkbox("", &show_description),
           ftxui::Renderer([&] {
             return ftxui::multiline_text(task_->description);
           }) | ftxui::Maybe(&show_description)}));
    }

    if (!task.dependencies.empty()) {
      ftxui::Elements blockers;
      for (auto id : task.dependencies)
        blockers.push_back(
            ftxui::text(std::format("{:3} {}", id, get_title(id))));

      result.push_back(ftxui::Renderer([=] {
        return ftxui::window(ftxui::text("Dependencies"),
                             ftxui::vbox(blockers));
      }));
    }

    if (!task.requirements.empty()) {
      ftxui::Elements blockers;
      for (auto id : task.requirements)
        blockers.push_back(
            ftxui::text(std::format("{:3} {}", id, get_group_name(id))));

      result.push_back(ftxui::Renderer([=] {
        return ftxui::window(ftxui::text("Requirements"),
                             ftxui::vbox(blockers));
      }));
    }

    if (task.after) {
      //  red when blocking?
      result.push_back(ftxui::Renderer([&] {
        return ftxui::window(
            ftxui::text("After"),
            // TODO use formatter calendar after it has landed.
            ftxui::text(std::format("{}.{:02}.{:02}",
                                    static_cast<int>(task.after->year()),
                                    static_cast<unsigned>(task.after->month()),
                                    static_cast<unsigned>(task.after->day()))));
      }));
    }

    widget = ftxui::Container::Vertical(result) | ftxui::border;
  }

  const task *task_;
  bool show_description{false};
  ftxui::Component widget;
};

struct column {
  column(const std::vector<ticket> tickets) {
    components.reserve(tickets.size());
    std::ranges::copy(tickets |
                          std::views::transform([](const gui::ticket &ticket) {
                            return ticket.widget;
                          }),
                      std::back_inserter(components));
    widget = ftxui::Container::Vertical(components) |
             ftxui::Maybe([&] { return !components.empty(); });
  }

  ftxui::Components components;
  ftxui::Component widget;
};

} // namespace gui

ftxui::Element create_widget(const task &task) {
  ftxui::Elements result;

  result.push_back(ftxui::text(
      task.project ? std::format("{:3} [{}] {}", task.id,
                                 get_project_name(task.project), task.title)
                   : std::format("{:3} {}", task.id, task.title)));

  if (!task.description.empty()) {
    if (task.status == task::tstatus::progress)
      result.push_back(ftxui::multiline_text(task.description));
  }

  if (!task.dependencies.empty()) {
    ftxui::Elements blockers;
    for (auto id : task.dependencies)
      blockers.push_back(
          ftxui::text(std::format("{:3} {}", id, get_title(id))));

    result.push_back(
        ftxui::window(ftxui::text("Blockers"), ftxui::vbox(blockers)));
  }

  return ftxui::vbox(result) | ftxui::border;
}

int main(int argc, const char *argv[]) {
  char *home = std::getenv("HOME");
  std::ifstream file{home + std::string{"/kaban"}};
  parse(file);
  std::cout << "Found " << tasks.size() << " tasks\n";

  std::vector<gui::ticket> inactive;
  std::vector<gui::ticket> blocked;
  std::vector<gui::ticket> backlog;
  std::vector<gui::ticket> progress;
  std::vector<gui::ticket> review;

  inactive.reserve(tasks.size());
  blocked.reserve(tasks.size());
  backlog.reserve(tasks.size());
  progress.reserve(tasks.size());
  review.reserve(tasks.size());
  for (const auto &task : tasks) {
    if (task.status == task::tstatus::backlog) {
      if (is_blocked(task))
        blocked.emplace_back(task);
      else if (!is_active(task))
        inactive.emplace_back(task);
      else
        backlog.emplace_back(task);
    } else if (task.status == task::tstatus::progress)
      progress.emplace_back(task);
    else if (task.status == task::tstatus::review)
      review.emplace_back(task);
  }

  bool enable_all = false;
  bool enable_refinement = false;
  std::array<bool, 5> enable_columns{false, false, true, true, true};

  gui::column inactive_column{inactive};
  gui::column blocked_column{blocked};
  gui::column backlog_column{backlog};
  gui::column progress_column{progress};
  gui::column review_column{review};

  ftxui::Component board = ftxui::Container::Vertical({
      ftxui::Checkbox(std::format("All ({}/{})", tasks.size(), tasks.size()),
                      &enable_all),
      ftxui::Checkbox(
          std::format("Refinement ({}/{})",
                      inactive.size() + blocked.size() + backlog.size(),
                      tasks.size()),
          &enable_refinement),
      ftxui::Container::Horizontal(
          {ftxui::Checkbox(
               std::format("Inactive ({}/{})", inactive.size(), tasks.size()),
               &enable_columns[0]),
           ftxui::Checkbox(
               std::format("Blocked ({}/{})", blocked.size(), tasks.size()),
               &enable_columns[1]),
           ftxui::Checkbox(
               std::format("Backlog ({}/{})", backlog.size(), tasks.size()),
               &enable_columns[2]),
           ftxui::Checkbox(std::format("In progress ({}/{})", progress.size(),
                                       tasks.size()),
                           &enable_columns[3]),
           ftxui::Checkbox(
               std::format("In review ({}/{})", review.size(), tasks.size()),
               &enable_columns[4])}),
      ftxui::Container::Horizontal({
          // Inactive
          ftxui::Container::Vertical(
              {ftxui::Renderer(inactive_column.widget,
                               [&] {
                                 return ftxui::window(
                                     ftxui::text("Inactive"),
                                     inactive_column.widget->Render());
                               })}) |
              ftxui::Maybe([&] {
                return enable_all | enable_refinement | enable_columns[0];
              }),
          // Blocked
          ftxui::Container::Vertical(
              {ftxui::Renderer(blocked_column.widget,
                               [&] {
                                 return ftxui::window(
                                     ftxui::text("Blocked"),
                                     blocked_column.widget->Render());
                               })}) |
              ftxui::Maybe([&] {
                return enable_all | enable_refinement | enable_columns[1];
              }),

          // Backlog
          ftxui::Container::Vertical(
              {ftxui::Renderer(backlog_column.widget,
                               [&] {
                                 return ftxui::window(
                                     ftxui::text("Backlog"),
                                     backlog_column.widget->Render());
                               })}) |
              ftxui::Maybe([&] {
                return enable_all | enable_refinement | enable_columns[2];
              }),
          // Progress
          ftxui::Container::Vertical(
              {ftxui::Renderer(progress_column.widget,
                               [&] {
                                 return ftxui::window(
                                     ftxui::text("In progress"),
                                     progress_column.widget->Render());
                               })}) |
              ftxui::Maybe([&] { return enable_all | enable_columns[3]; }),
          // Review
          ftxui::Container::Vertical(
              {ftxui::Renderer(review_column.widget,
                               [&] {
                                 return ftxui::window(
                                     ftxui::text("In Review"),
                                     (review_column.widget)->Render());
                               })}) |
              ftxui::Maybe([&] { return enable_all | enable_columns[4]; }) //
      })                                                                   //
  });

  ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
  screen.Loop(ftxui::Container::Vertical(
                  {ftxui::Button("Quit", screen.ExitLoopClosure()), //
                   board})                                          //
              | ftxui::xflex |
              ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 40) |
              ftxui::border);
}
