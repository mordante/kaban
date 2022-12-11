#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

struct task {
  std::string id;
  std::string title;
  std::string status;
  std::string blocked_by_tasks;
};

std::vector<task> parse(std::ifstream &file) {
  bool parse = false;

  std::vector<task> tasks;

  std::string line;
  while (std::getline(file, line)) {
    if (parse) {
      if (line == "") {
        parse = false;
        continue;
      }

      int pos = line.find('=');
      if (line.substr(0, pos) == "id")
        tasks.back().id = line.substr(pos + 1);
      if (line.substr(0, pos) == "title")
        tasks.back().title = line.substr(pos + 1);
      if (line.substr(0, pos) == "status")
        tasks.back().status = line.substr(pos + 1);
      if (line.substr(0, pos) == "blocked_by_tasks")
        tasks.back().blocked_by_tasks = line.substr(pos + 1);

    } else if (line == "[task]") {
      parse = true;
      tasks.emplace_back();
    }
  }
  return tasks;
}

bool is_blocked(const std::vector<task> &tasks, const task &task) {
  if (task.blocked_by_tasks.empty())
    return false;

  auto it =
      std::find_if(tasks.begin(), tasks.end(),
                   [id = task.id](const auto &task) { return task.id == id; });
  return it != tasks.end();
}

void print(std::string_view title, const std::vector<task> &tasks) {
  if (tasks.empty())
    return;

  std::cout << "[" << title << "]\n";
  for (const auto &task : tasks)
    std::cout << "- " << task.title << '\n';
  std::cout << '\n';
}

int main() {
  char *home = std::getenv("HOME");
  std::ifstream file{home + std::string{"/kaban"}};
  std::vector<task> tasks = parse(file);
  std::cout << "Found " << tasks.size() << " tasks\n";

  std::vector<task> blocked;
  std::vector<task> backlog;
  std::vector<task> progress;
  std::vector<task> review;
  for (const auto &task : tasks) {
    if (task.status == "backlog") {
      if (is_blocked(tasks, task))
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
  print("BLOCKED", blocked);
}
