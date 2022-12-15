#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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
  std::vector<std::size_t> blocked_by_tasks;

private:
  void parse(parser &parser);

  void validate() const {
    if (id == 0)
      throw 42;
    if (title.empty())
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
      else if (line.second[0] == "title")
        title = line.second[1];
      else if (line.second[0] == "description")
        description = line.second[1];
      else if (line.second[0] == "status")
        status = parse_status(line.second[1]);
      else if (line.second[0] == "blocked_by_tasks")
        blocked_by_tasks = parse_id_list(line.second[1]);
#if 0
      else
        throw 42;
#endif
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
#if 0
      else if (line.second[0] == "[group]")
        groups.emplace_back(file);
#endif
      else if (line.second[0] == "[task]")
        tasks.emplace_back(parser);
#if 0
      else
        throw 42;
#endif
      break;

    case parser::tresult::pair:
#if 0
      throw 42;
#else
      break;
#endif
    }
  }
}

bool is_complete(task::tstatus status) {
  return status == task::tstatus::done || status == task::tstatus::discarded;
}

bool is_blocked(const task &task) {
  for (auto id : task.blocked_by_tasks) {
    auto it = std::find_if(tasks.begin(), tasks.end(),
                           [id](const auto &task) { return task.id == id; });
    if (it == tasks.end())
      throw 42;

    if (!is_complete(it->status))
      return true;
  }
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
  auto it = std::find_if(projects.begin(), projects.end(),
                         [id](const auto &task) { return task.id == id; });

  if (it == projects.end())
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

    if (!task.blocked_by_tasks.empty())
      for (auto id : task.blocked_by_tasks)
        std::cout << std::format("      x {:3} {}\n", id, get_title(id));
  }
  std::cout << '\n';
}

std::shared_ptr<ftxui::Node> create_widget(const task &task) {
  ftxui::Elements result;

  result.push_back(ftxui::text(
      task.project ? std::format("{:3} [{}] {}", task.id,
                                 get_project_name(task.project), task.title)
                   : std::format("{:3} {}", task.id, task.title)));

  if (!task.description.empty()) {
    if (task.status == task::tstatus::progress)
      result.push_back(ftxui::paragraph(task.description));
  }

  if (!task.blocked_by_tasks.empty()) {
    ftxui::Elements blockers;
    for (auto id : task.blocked_by_tasks)
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

  ftxui::Elements inactive;
  ftxui::Elements blocked;
  ftxui::Elements backlog;
  ftxui::Elements progress;
  ftxui::Elements review;
  for (const auto &task : tasks) {
    if (task.status == task::tstatus::backlog) {
      if (is_blocked(task))
        blocked.push_back(create_widget(task));
      else if (!is_active(task))
        inactive.push_back(create_widget(task));
      else
        backlog.push_back(create_widget(task));
    } else if (task.status == task::tstatus::progress)
      progress.push_back(create_widget(task));
    else if (task.status == task::tstatus::review)
      review.push_back(create_widget(task));
  }

  bool enable_all = false;
  bool enable_refinement = false;
  std::array<bool, 5> enable_columns{false, false, true, true, true};

  auto board = [&] {
    ftxui::Elements columns;
    if (enable_all | enable_refinement | enable_columns[0])
      columns.push_back(ftxui::window(
          ftxui::text("Inactive"),
          ftxui::vbox(inactive))); // XXX example has not text for first element
    if (enable_all | enable_refinement | enable_columns[1])
      columns.push_back(
          ftxui::window(ftxui::text("Blocked"), ftxui::vbox(blocked)));

    if (enable_all | enable_refinement | enable_columns[2])
      columns.push_back(
          ftxui::window(ftxui::text("Backlog"), ftxui::vbox(backlog)));

    if (enable_all | enable_columns[3])
      columns.push_back(
          ftxui::window(ftxui::text("In progress"), ftxui::vbox(progress)) |
          ftxui::flex);

    if (enable_all | enable_columns[4])
      columns.push_back(
          ftxui::window(ftxui::text("In review"), ftxui::vbox(review)));

    return ftxui::hbox(columns);
  };

  auto screen = ftxui::ScreenInteractive::Fullscreen();

  auto quit = ftxui::Button("Quit", screen.ExitLoopClosure());
  auto show_all_columns = ftxui::Checkbox(
      std::format("All ({}/{})", tasks.size(), tasks.size()), &enable_all);
  auto show_refinement_columns = ftxui::Checkbox(
      std::format("Refinement ({}/{})",
                  inactive.size() + blocked.size() + backlog.size(),
                  tasks.size()),
      &enable_refinement);
  auto show_column_inactive = ftxui::Checkbox(
      std::format("Inactive ({}/{})", inactive.size(), tasks.size()),
      &enable_columns[0]);
  auto show_column_blocked = ftxui::Checkbox(
      std::format("Blocked ({}/{})", blocked.size(), tasks.size()),
      &enable_columns[1]);
  auto show_column_backlog = ftxui::Checkbox(
      std::format("Backlog ({}/{})", backlog.size(), tasks.size()),
      &enable_columns[2]);
  auto show_column_progress = ftxui::Checkbox(
      std::format("In progress ({}/{})", progress.size(), tasks.size()),
      &enable_columns[3]);
  auto show_column_review = ftxui::Checkbox(
      std::format("In review ({}/{})", review.size(), tasks.size()),
      &enable_columns[4]);

  // Note this seems a bit a duplicate of the component, not sure why.
  auto layout = ftxui::Container::Vertical(
      {quit, show_all_columns, show_refinement_columns, show_column_inactive,
       show_column_blocked, show_column_backlog, show_column_progress,
       show_column_review});

  auto component = ftxui::Renderer(layout, [&] {
    return ftxui::vbox({
               quit->Render(),                    //
               show_all_columns->Render(),        //
               show_refinement_columns->Render(), //
               ftxui::hbox({
                   show_column_inactive->Render(), //
                   show_column_blocked->Render(),  //
                   show_column_backlog->Render(),  //
                   show_column_progress->Render(), //
                   show_column_review->Render(),   //
               }),                                 //
               ftxui::separator(),                 //
               board()                             //
           }) |
           ftxui::xflex | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 40) |
           ftxui::border;
  });

  screen.Loop(component);
}
