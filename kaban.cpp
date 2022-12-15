#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
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

struct project {
  explicit project(std::istream &stream) {
    parse(stream);
    validate();
  }
  std::size_t id;
  std::string name;
  std::string description;
  bool active{true};

private:
  void parse(std::istream &stream);

  void validate() const {
    if (id == 0)
      throw 42;
    if (name.empty())
      throw 42;
  }
};

struct task {

  explicit task(std::istream &stream) {
    parse(stream);
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
  void parse(std::istream &stream);

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

void project::parse(std::istream &stream) {
  std::string line;
  while (std::getline(stream, line)) {
    if (line.empty())
      return;

    int pos = line.find('=');
    if (line.substr(0, pos) == "id")
      id = parse_id(line.substr(pos + 1));
    else if (line.substr(0, pos) == "name")
      name = line.substr(pos + 1);
    else if (line.substr(0, pos) == "description")
      description = line.substr(pos + 1);
    else if (line.substr(0, pos) == "active")
      active = parse_bool(line.substr(pos + 1));
    else
      throw 42;
  }
}

void task::parse(std::istream &stream) {
  std::string line;
  while (std::getline(stream, line)) {
    if (line.empty())
      return;

    int pos = line.find('=');
    if (line.substr(0, pos) == "id")
      id = parse_id(line.substr(pos + 1));
    else if (line.substr(0, pos) == "project")
      project = parse_id(line.substr(pos + 1));
    else if (line.substr(0, pos) == "title")
      title = line.substr(pos + 1);
    else if (line.substr(0, pos) == "description")
      description = line.substr(pos + 1);
    else if (line.substr(0, pos) == "status")
      status = parse_status(line.substr(pos + 1));
    else if (line.substr(0, pos) == "blocked_by_tasks")
      blocked_by_tasks = parse_id_list(line.substr(pos + 1));
  }
}
void parse(std::ifstream &file) {
  std::string line;
  while (std::getline(file, line)) {
    if (line == "[project]")
      projects.emplace_back(file);
#if 0
    else if (line == "[group]")
      groups.emplace_back(file);
#endif
    else if (line == "[task]")
      tasks.emplace_back(file);
#if 0
    else if (!line.empty())
      throw 42;
#endif
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

int main(int argc, const char *argv[]) {
  char *home = std::getenv("HOME");
  std::ifstream file{home + std::string{"/kaban"}};
  parse(file);
  std::cout << "Found " << tasks.size() << " tasks\n";

  std::vector<task> inactive;
  std::vector<task> blocked;
  std::vector<task> backlog;
  std::vector<task> progress;
  std::vector<task> review;
  for (const auto &task : tasks) {
    if (task.status == task::tstatus::backlog) {
      if (is_blocked(task))
        blocked.push_back(task);
      else if (!is_active(task))
        inactive.push_back(task);
      else
        backlog.push_back(task);
    } else if (task.status == task::tstatus::progress)
      progress.push_back(task);
    else if (task.status == task::tstatus::review)
      review.push_back(task);
  }
  print("REVIEW", review);
  print("PROGRESS", progress);
  print("BACKLOG", backlog);
  if (argc == 2 && strcmp(argv[1], "-b") == 0) {
    print("BLOCKED", blocked);
    print("INACTIVE", inactive);
  }
}
