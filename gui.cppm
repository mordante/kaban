module;
#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

export module gui;
import data;
import stl;

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

ftxui::Element create_label(std::string text, std::string_view color) {
  text = "[" + text + "]";
  if (color.empty())
    return ftxui::text(text);

  return ftxui::bgcolor(to_color(color), ftxui::text(text));
}

ftxui::Component create_title(const data::ttask *task) {
  return ftxui::Renderer([=] {
    ftxui::Elements result;
    result.push_back(ftxui::text(std::format("{:3} ", task->id)));

    if (size_t project_id =
            task->group ? data::get_group(task->group).project : task->project;
        project_id) {

      const data::tproject &project = data::get_project(project_id);
      result.push_back(create_label(project.name, project.color));
    }

    if (task->group) {
      const data::tgroup &group = data::get_group(task->group);
      result.push_back(create_label(group.name, group.color));
    }

    // TODO ugly spacing hack.
    result.push_back(ftxui::text(" "));
    result.push_back(ftxui::text(task->title));

    if (task->labels.empty())
      return ftxui::hflow(result);

    ftxui::Elements labels;
    for (auto &id : task->labels) {
      const data::tlabel &label = data::get_label(id);
      labels.push_back(create_label(label.name, label.color));
    }

    return ftxui::vbox(ftxui::hflow(result), ftxui::hflow(labels));
  });
}

class tticket final : public ftxui::ComponentBase {
public:
  explicit tticket(const data::ttask *task) : task_(task) {

    ftxui::Components result;
    result.push_back(create_title(task_));
    if (!task_->description.empty()) {
      show_description = task_->status == data::ttask::tstatus::progress;
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
            ftxui::text(std::format("{:%Y.%m.%d}", *task_->after)));
      }));
    }

    widget_ = ftxui::Container::Vertical(result) | ftxui::border;
  }

  ftxui::Element Render() override { return widget_->Render(); }

  bool OnEvent(ftxui::Event event) override { return widget_->OnEvent(event); }

  bool Focusable() const override { return true; }

private:
  const data::ttask *task_;
  bool show_description{false};
  ftxui::Component widget_;
};

ftxui::Components
create_tickets(const std::vector<const data::ttask *> &tasks) {
  ftxui::Components result;
  for (const auto *task : tasks)
    result.emplace_back(std::make_shared<tticket>(task));

  return result;
}

enum tcolumn_index {
  inactive = 0,
  blocked,
  backlog,
  selected,
  progress,
  review,
  done,
  discarded
};

constexpr std::size_t column_count = 8;

constexpr std::array<std::string_view, column_count> column_names = {
    "Inactive",    "Blocked",   "Backlog", "Selected",
    "In progress", "In review", "Done",    "Discarded"};

tcolumn_index get_column_index(const data::ttask &task) {
  switch (task.status) {
  case data::ttask::tstatus::backlog:
    if (!data::is_active(task))
      return inactive;

    if (data::is_blocked(task))
      return blocked;

    return backlog;

  case data::ttask::tstatus::selected:
    return selected;

  case data::ttask::tstatus::progress:
    return progress;

  case data::ttask::tstatus::review:
    return review;

  case data::ttask::tstatus::done:
    return done;

  case data::ttask::tstatus::discarded:
    return discarded;
  }
}

class tboard final : public ftxui::ComponentBase {
public:
  tboard() { load_tasks(); }

  ftxui::Element Render() override {

    // Note this function "digs up" the elements from the internal container.
    // This gives a lot of hard-coded magic numbers. This should be improved.

    ftxui::Elements columns;
    for (std::size_t i = 0; i < column_count; ++i)
      if (column_visibility_[i]())
        columns.emplace_back(ftxui::window(
            ftxui::text(std::string(column_names[i])),
            ChildAt(0)->ChildAt(1)
                        ->ChildAt(i)
                        ->ChildAt(0) // maybe
                        ->ChildAt(0) // maximum width
                        ->ChildAt(0) // minimum width
                        ->ChildCount() == 0
                ? ftxui::filler() | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 20)
                : ChildAt(0)->ChildAt(1)->ChildAt(i)->Render()));

    return ftxui::vbox({
        ChildAt(0)->ChildAt(0)->ChildAt(0)->Render(),
        ftxui::hflow({
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(0)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(1)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(2)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(3)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(4)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(5)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(6)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(7)->Render(),
        }),
        ftxui::hbox(columns),
    });
  }

private:
  void load_tasks() {
    std::array<std::vector<ftxui::Component>, column_count> columns;
    // Creates a ticket for every task and stores the shared pointer in two
    // places:
    // - tickets_ as a ticket
    // - columns as a component, this will be used further in this function.
    for (const auto &task : data::get_state().tasks)
      columns[get_column_index(task)].emplace_back(tickets_.emplace_back(
          std::make_shared<tticket>(std::addressof(task))));

    Add(ftxui::Container::Vertical(
        {create_column_buttons(columns), create_columns(columns)}));
  }

  ftxui::Component create_column_buttons(
      const std::array<ftxui::Components, column_count> &columns) {

    ftxui::Components column_buttons;
    for (int i = 0; i < column_count; ++i) // zip view
      column_buttons.emplace_back(
          ftxui::Checkbox(std::format("{} ({}/{}))", column_names[i],
                                      columns[i].size(), tickets_.size()),
                          std::addressof(visible_[i])));

    return ftxui::Container::Vertical({
        ftxui::Container::Horizontal({
            ftxui::Checkbox(
                std::format("All ({}/{})", tickets_.size(), tickets_.size()),
                std::addressof(all_visible_)),
            ftxui::Checkbox(
                std::format("Refinement ({}/{})",
                            std::accumulate(
                                columns.begin(), columns.begin() + progress, 0,
                                [](size_t init, const auto &column) {
                                  return init + column.size();
                                }),
                            tickets_.size()),
                std::addressof(refinement_visible_))   //
        }),                                            //
        ftxui::Container::Horizontal({column_buttons}) //
    });                                                //
  }

  ftxui::Component
  create_columns(std::array<ftxui::Components, column_count> tickets) {
    ftxui::Components columns;
    for (int i = 0; i < column_count; ++i) // zip view
      columns.emplace_back(
          ftxui::Container::Vertical({std::move(tickets[i])})  //
          | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 19) //
          | ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 67)    //
          | ftxui::Maybe(column_visibility_[i])                //
      );

    return ftxui::Container::Horizontal({columns});
  }

  std::vector<std::shared_ptr<tticket>> tickets_;

  bool dirty_{true};

  bool all_visible_{false};
  bool refinement_visible_{false};
  std::array<bool, column_count> visible_{false, false, true,  true,
                                          true,  true,  false, false};

  std::array<std::function<bool()>, column_count> column_visibility_{
      [&] { return all_visible_ | refinement_visible_ | visible_[0]; },
      [&] { return all_visible_ | refinement_visible_ | visible_[1]; },
      [&] { return all_visible_ | refinement_visible_ | visible_[2]; },
      [&] { return all_visible_ | refinement_visible_ | visible_[3]; },
      [&] { return all_visible_ | visible_[4]; },
      [&] { return all_visible_ | visible_[5]; },
      [&] { return all_visible_ | visible_[6]; },
      [&] { return all_visible_ | visible_[7]; },
  };
};

class tlabel final : public ftxui::ComponentBase {
public:
  explicit tlabel(const data::tlabel *label) {
    Add(ftxui::Renderer([=] {
      ftxui::Elements elements;
      elements.emplace_back(ftxui::text(std::format("{:3} ", label->id)));
      elements.emplace_back(ftxui::text((label->name)));
      if (!label->description.empty())
        elements.emplace_back(ftxui::text(label->description) | ftxui::border);
      if (!label->color.empty())
        ftxui::bgcolor(to_color(label->color), ftxui::text(label->color));
      return ftxui::vbox({elements});
    }));
  }
};

class tconfiguration final : public ftxui::ComponentBase {
public:
  tconfiguration() { load_configuration(); }

  ftxui::Element Render() override {
    ftxui::Elements columns;
    if (!labels_.empty())
      columns.emplace_back(
          ftxui::window(ftxui::text("Labels"), ftxui::vbox(labels_)) |
          ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 19) |
          ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 67));

    return ftxui::hbox(columns);

#if 0
    // Note this function "digs up" the elements from the internal container.
    // This gives a lot of hard-coded magic numbers. This should be improved.

    ftxui::Elements columns;
    for (std::size_t i = 0; i < column_count; ++i)
      if (column_visibility_[i]())
        columns.emplace_back(ftxui::window(
            ftxui::text(std::string(column_names[i])),
            ChildAt(0)->ChildAt(1)
                        ->ChildAt(i)
                        ->ChildAt(0) // maybe
                        ->ChildAt(0) // maximum width
                        ->ChildAt(0) // minimum width
                        ->ChildCount() == 0
                ? ftxui::filler() | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 20)
                : ChildAt(0)->ChildAt(1)->ChildAt(i)->Render()));

    return ftxui::vbox({
        ChildAt(0)->ChildAt(0)->ChildAt(0)->Render(),
        ftxui::hflow({
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(0)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(1)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(2)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(3)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(4)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(5)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(6)->Render(),
            ChildAt(0)->ChildAt(0)->ChildAt(1)->ChildAt(7)->Render(),
        }),
        ftxui::hbox(columns),
    });
#endif
  }

private:
  void load_configuration() {
    ftxui::Components labels;
    for (const auto &label : data::get_state().labels)
      labels.emplace_back(labels_.emplace_back(
          std::make_shared<tlabel>(std::addressof(label))));

    Add(ftxui::Container::Horizontal({
        ftxui::Container::Vertical(std::move(labels)),
    }));
  }

  std::vector<std::shared_ptr<tlabel>> labels_;
};

export namespace gui {

ftxui::Component board() { return std::make_shared<tboard>(); }
ftxui::Component configuration() { return std::make_shared<tconfiguration>(); }

} // namespace gui
