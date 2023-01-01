module;
#include <algorithm>
#include <expected>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

export module gui;
import data;
import stl;

export namespace gui {
enum class tcolumn {
  inactive = 0,
  blocked,
  backlog,
  selected,
  progress,
  review,
  done,
  discarded
};

tcolumn get_column(const task &task) {
  switch (task.status) {
  case task::tstatus::backlog:
    if (!data::is_active(task))
      return tcolumn::inactive;

    if (data::is_blocked(task))
      return tcolumn::blocked;

    return tcolumn::backlog;

  case task::tstatus::selected:
    return tcolumn::selected;

  case task::tstatus::progress:
    return tcolumn::progress;

  case task::tstatus::review:
    return tcolumn::review;

  case task::tstatus::done:
    return tcolumn::done;

  case task::tstatus::discarded:
    return tcolumn::discarded;
  }
}
} // namespace gui

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

ftxui::Element multiline_text(const std::string &the_text) {
  ftxui::Elements output;
  std::stringstream ss(the_text);
  std::string line;
  while (std::getline(ss, line)) {
    output.push_back(ftxui::paragraph(line));
  }
  return ftxui::vbox(output);
}

ftxui::Element create_labelX(std::string text, std::string_view color) {
  text = "[" + text + "]";
  if (color.empty())
    return ftxui::text(text);

  return ftxui::bgcolor(to_color(color), ftxui::text(text));
}

ftxui::Component create_title(const task *task) {
  return ftxui::Renderer([=] {
    ftxui::Elements result;
    result.push_back(ftxui::text(std::format("{:3} ", task->id)));

    if (size_t project_id =
            task->group ? data::get_group(task->group).project : task->project;
        project_id) {

      const project &project = data::get_project(project_id);
      result.push_back(create_labelX(project.name, project.color));
    }

    if (task->group) {
      const group &group = data::get_group(task->group);
      result.push_back(create_labelX(group.name, group.color));
    }

    // TODO ugly spacing hack.
    result.push_back(ftxui::text(" "));
    result.push_back(ftxui::text(task->title));

    if (task->labels.empty())
      return ftxui::hflow(result);

    ftxui::Elements labels;
    for (auto &id : task->labels) {
      const label &label = data::get_label(id);
      labels.push_back(create_labelX(label.name, label.color));
    }

    return ftxui::vbox(ftxui::hflow(result), ftxui::hflow(labels));
  });
}

class tticket final : public ftxui::ComponentBase {
public:
  explicit tticket(const task *task) : task_(task) {

    ftxui::Components result;
    result.push_back(create_title(task_));
    if (!task_->description.empty()) {
      show_description = task_->status == task::tstatus::progress;
      result.push_back(ftxui::Container::Horizontal(
          {ftxui::Checkbox("", &show_description),
           ftxui::Renderer([&] { return multiline_text(task_->description); }) |
               ftxui::Maybe(&show_description)}));
    }

    if (!task_->dependencies.empty()) {
      ftxui::Elements blockers;
      for (auto id : task_->dependencies)
        blockers.push_back(
            ftxui::text(std::format("{:3} {}", id, data::get_task(id).title)));

      result.push_back(ftxui::Renderer([=] {
        return ftxui::window(ftxui::text("Dependencies"),
                             ftxui::vbox(blockers));
      }));
    }

    if (!task_->requirements.empty()) {
      ftxui::Elements blockers;
      for (auto id : task_->requirements)
        blockers.push_back(
            ftxui::text(std::format("{:3} {}", id, data::get_group(id).name)));

      result.push_back(ftxui::Renderer([=] {
        return ftxui::window(ftxui::text("Requirements"),
                             ftxui::vbox(blockers));
      }));
    }

    if (task_->after) {
      //  red when blocking?
      result.push_back(ftxui::Renderer([&] {
        return ftxui::window(
            ftxui::text("After"),
            // TODO use formatter calendar after it has landed.
            ftxui::text(std::format(
                "{}.{:02}.{:02}", static_cast<int>(task_->after->year()),
                static_cast<unsigned>(task_->after->month()),
                static_cast<unsigned>(task_->after->day()))));
      }));
    }

    widget_ = ftxui::Container::Vertical(result) | ftxui::border;
  }

  ftxui::Element Render() override { return widget_->Render(); }

  bool OnEvent(ftxui::Event event) override { return widget_->OnEvent(event); }

  bool Focusable() const override { return true; }

private:
  const task *task_;
  bool show_description{false};
  ftxui::Component widget_;
};

ftxui::Components create_tickets(const std::vector<const task *> &tasks) {
  ftxui::Components result;
  for (const auto *task : tasks)
    result.emplace_back(std::make_shared<tticket>(task));

  return result;
}

export namespace gui {

class tfoo final : public ftxui::ComponentBase {
public:
  explicit tfoo(const std::string_view title, std::function<bool()> visible,
                float *scrollbar)
      : title_(ftxui::text(std::string(title))), visible_(visible),
        scrollbar_(scrollbar) {}

  void add(const task &task) {
    tasks_.push_back(std::addressof(task));
    dirty_ = true;
  }

  ftxui::Element Render() override {
    if (dirty_ && !tasks_.empty()) {
      widget_ = ftxui::Container::Vertical(create_tickets(tasks_));
      dirty_ = false;
    }
    return (ftxui::Renderer([&] {
              return ftxui::window(title_,
                                   tasks_.empty() ? ftxui::filler()
                                                  : widget_->Render()) //
                     | ftxui::vscroll_indicator                        //
                     | ftxui::focusPositionRelative(0, *scrollbar_)    //
                     | ftxui::yframe                                   //
                     | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 19);
            }) //
            | ftxui::Maybe(visible_))
        ->Render();
  }

  bool OnEvent(ftxui::Event event) override {
    if (tasks_.empty())
      return false;
    return widget_->OnEvent(event);
  }

  bool Focusable() const override { return true; }

private:
  ftxui::Element title_;
  std::function<bool()> visible_;
  float *scrollbar_;
  std::vector<const task *> tasks_;
  bool dirty_{true};
  ftxui::Component widget_;
};

} // namespace gui
