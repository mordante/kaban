#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <expected>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <sstream> // multiline

import data;

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
    return std::chrono::system_clock::now() <=
           static_cast<std::chrono::sys_days>(*task.after);
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

const project &get_project(std::size_t id) {
  auto it =
      std::find_if(projects.begin(), projects.end(),
                   [id](const auto &project) { return project.id == id; });

  if (it == projects.end())
    throw 42;
  return *it;
}

std::string_view get_project_name(std::size_t id) {
  return get_project(id).name;
}

const label &get_label(std::size_t id) {
  auto it = std::find_if(labels.begin(), labels.end(),
                         [id](const auto &label) { return label.id == id; });

  if (it == labels.end())
    throw 42;
  return *it;
}

const group &get_group(std::size_t id) {
  auto it = std::find_if(groups.begin(), groups.end(),
                         [id](const auto &group) { return group.id == id; });

  if (it == groups.end())
    throw 42;
  return *it;
}

std::string_view get_group_name(std::size_t id) { return get_group(id).name; }

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

// position x y is the end of the visible area
Decorator xfocusPosition(int x) { return focusPosition(x, 0); }
Decorator yfocusPosition(int y) { return focusPosition(0, y); }
Decorator xfocusPositionRelative(float x) {
  return focusPositionRelative(x, 0.0f);
}
Decorator yfocusPositionRelative(float y) {
  return focusPositionRelative(0.0f, y);
}

} // namespace ftxui

namespace gui {

ftxui::Color to_color(std::string_view input) {
  if (input == "RED")
    return ftxui::Color::Red;
  if (input == "GREEN")
    return ftxui::Color::Green;
  if (input == "YELLOW")
    return ftxui::Color::Yellow;
  if (input == "BLUE")
    return ftxui::Color::Blue;
  if (input == "MAGENTA")
    return ftxui::Color::Magenta;
  if (input == "CYAN")
    return ftxui::Color::Cyan;
  if (input == "red")
    return ftxui::Color::RedLight;
  if (input == "green")
    return ftxui::Color::GreenLight;
  if (input == "yellow")
    return ftxui::Color::YellowLight;
  if (input == "blue")
    return ftxui::Color::BlueLight;
  if (input == "magenta")
    return ftxui::Color::MagentaLight;
  if (input == "cyan")
    return ftxui::Color::CyanLight;

  throw 42;
}

ftxui::Element create_label(std::string text, std::string_view color) {
  text = "[" + text + "]";
  if (color.empty())
    return ftxui::text(text);

  return ftxui::bgcolor(to_color(color), ftxui::text(text));
}

struct ticket {
  ticket(const task &task) : task_(std::addressof(task)) {

    ftxui::Components result;

    result.push_back(ftxui::Renderer([&] {
      // TODO ugly spacing hack.
      ftxui::Elements result;
      result.push_back(ftxui::text(std::format("{:3} ", task.id)));

      if (size_t project_id =
              task.group ? get_group(task.group).project : task.project;
          project_id) {

        const project &project = get_project(project_id);
        result.push_back(create_label(project.name, project.color));
      }

      if (task.group) {
        const group &group = get_group(task.group);
        result.push_back(create_label(group.name, group.color));
      }

      // TODO ugly spacing hack.
      result.push_back(ftxui::text(" "));
      result.push_back(ftxui::text(task.title));

      if (task.labels.empty())
        return ftxui::hflow(result);

      ftxui::Elements labels;
      for (auto &id : task.labels) {
        const label &label = get_label(id);
        labels.push_back(create_label(label.name, label.color));
      }

      return ftxui::vbox(ftxui::hflow(result), ftxui::hflow(labels));
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
#if 0
  parse(file);
#else

  std::string input{std::istreambuf_iterator<char>(file), {}};
  std::expected<data::tstate *, data::tparse_error *> result =
      data::parse(input);
  if (!result) {
    data::tparse_error &error = *result.error();
    std::cerr << std::format(R"(Failed parsing
{}:{}
{}
{}
)",
                             home + std::string{"/kaban"}, error.line_no,
                             error.line, error.message);

    return EXIT_FAILURE;
  }

  std::unique_ptr<data::tstate> state{*result};
  labels = state->labels;
  projects = state->projects;
  groups = state->groups;
  tasks = state->tasks;
#endif

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
  std::array<float, 5> position{0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

  gui::column inactive_column{inactive};
  gui::column blocked_column{blocked};
  gui::column backlog_column{backlog};
  gui::column progress_column{progress};
  gui::column review_column{review};

  ftxui::Component board = ftxui::Container::Vertical({

      ftxui::Slider("Inactive   ", &position[0], 0.f, 1.f, 0.01f),
      ftxui::Slider("Blocked    ", &position[1], 0.f, 1.f, 0.01f),
      ftxui::Slider("Backlog    ", &position[2], 0.f, 1.f, 0.01f),
      ftxui::Slider("In progress", &position[3], 0.f, 1.f, 0.01f),
      ftxui::Slider("In review  ", &position[4], 0.f, 1.f, 0.01f),

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
                                            inactive_column.widget->Render()) |
                                        ftxui::vscroll_indicator |
                                        ftxui::yfocusPositionRelative(
                                            position[0]) |
                                        ftxui::yframe;
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
                                     blocked_column.widget->Render()      //
                                         | ftxui::vscroll_indicator       //
                                         | ftxui::yfocusPositionRelative( //
                                               position[1])               //
                                         | ftxui::yframe);
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
                                     backlog_column.widget->Render()      //
                                         | ftxui::vscroll_indicator       //
                                         | ftxui::yfocusPositionRelative( //
                                               position[2])               //
                                         | ftxui::yframe);
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
                                     progress_column.widget->Render()     //
                                         | ftxui::vscroll_indicator       //
                                         | ftxui::yfocusPositionRelative( //
                                               position[3])               //
                                         | ftxui::yframe);
                               })}) |
              ftxui::Maybe([&] { return enable_all | enable_columns[3]; }),
          // Review
          ftxui::Container::Vertical(
              {ftxui::Renderer(review_column.widget,
                               [&] {
                                 return ftxui::window(
                                            ftxui::text("In Review"),
                                            (review_column.widget)->Render()) //
                                        | ftxui::vscroll_indicator            //
                                        | ftxui::yfocusPositionRelative(      //
                                              position[4])                    //
                                        | ftxui::yframe;
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
