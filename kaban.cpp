#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

struct task {
  std::size_t id;
  std::string title;
  std::string status;
  std::vector<std::size_t> blocked_by_tasks;
};

std::vector<task> tasks;

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

void parse(std::ifstream &file) {
  bool parse = false;

  std::string line;
  while (std::getline(file, line)) {
    if (parse) {
      if (line == "") {
        parse = false;
        continue;
      }

      int pos = line.find('=');
      if (line.substr(0, pos) == "id")
        tasks.back().id = parse_id(line.substr(pos + 1));
      if (line.substr(0, pos) == "title")
        tasks.back().title = line.substr(pos + 1);
      if (line.substr(0, pos) == "status")
        tasks.back().status = line.substr(pos + 1);
      if (line.substr(0, pos) == "blocked_by_tasks")
        tasks.back().blocked_by_tasks = parse_id_list(line.substr(pos + 1));

    } else if (line == "[task]") {
      parse = true;
      tasks.emplace_back();
    }
  }
}

bool is_blocked(const task &task) {
  if (task.blocked_by_tasks.empty())
    return false;

  auto it =
      std::find_if(tasks.begin(), tasks.end(),
                   [id = task.id](const auto &task) { return task.id == id; });
  return it != tasks.end();
}

std::string_view get_title(std::size_t id) {
  auto it = std::find_if(tasks.begin(), tasks.end(),
                         [id](const auto &task) { return task.id == id; });

  if (it == tasks.end())
    throw 42;

  return it->title;
}

void print(std::string_view title, const std::vector<task> &tasks) {
  if (tasks.empty())
    return;

  std::cout << std::format("[{}]\n", title);
  for (const auto &task : tasks) {
    std::cout << std::format("- {:3} {}\n", task.id, task.title);

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

  std::vector<task> blocked;
  std::vector<task> backlog;
  std::vector<task> progress;
  std::vector<task> review;
  for (const auto &task : tasks) {
    if (task.status == "backlog") {
      if (is_blocked(task))
        blocked.push_back(task);
      else
        backlog.push_back(task);
    } else if (task.status == "progress")
      progress.push_back(task);
    else if (task.status == "review")
      review.push_back(task);
  }
  print("REVIEW", review);
  print("PROGRESS", progress);
  print("BACKLOG", backlog);
  if (argc == 2 && strcmp(argv[1], "-b") == 0)
    print("BLOCKED", blocked);
}
