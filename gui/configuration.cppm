export module gui:configuration;
import :helpers;

import ftxui;
import data;
import std;

static ftxui::Element create_title(std::size_t id, const std::string &name,
                                   data::tcolor color) {
  return ftxui::hbox({ftxui::text(std::format("{:3} ", id)),
                      detail::create_text(name, color)});
}

class tlabel final : public ftxui::ComponentBase {
public:
  explicit tlabel(const data::tlabel *label) {
    Add(ftxui::Renderer([=] {
      ftxui::Elements elements;
      elements.emplace_back(create_title(label->id, label->name, label->color));
      if (!label->description.empty())
        elements.emplace_back(ftxui::text(label->description));
      return ftxui::vbox({elements}) | ftxui::border;
    }));
  }
};

class tproject final : public ftxui::ComponentBase {
public:
  explicit tproject(const data::tproject *project)
      : active_(ftxui::Checkbox(
            // TODO const_cast is a hack
            "Active", const_cast<bool *>(std::addressof(project->active)))) {
    Add(ftxui::Renderer(active_, [=] {
      ftxui::Elements elements;
      elements.emplace_back(
          create_title(project->id, project->name, project->color));
      if (!project->description.empty())
        elements.emplace_back(ftxui::text(project->description));
      elements.emplace_back(active_->Render());
      return ftxui::vbox({elements}) | ftxui::border;
    }));
  }

private:
  ftxui::Component active_;
};

class tgroup final : public ftxui::ComponentBase {
public:
  explicit tgroup(const data::tgroup *group)
      : active_(ftxui::Checkbox(
            // TODO const_cast is a hack
            "Active", const_cast<bool *>(std::addressof(group->active)))) {
    Add(ftxui::Renderer(active_, [=] {
      ftxui::Elements elements;
      elements.emplace_back(create_title(group->id, group->name, group->color));
      {
        const data::tproject &project = data::get_project(group->project);
        elements.emplace_back(
            create_title(project.id, project.name, project.color));
      }
      if (!group->description.empty())
        elements.emplace_back(ftxui::text(group->description));
      elements.emplace_back(active_->Render());
      return ftxui::vbox({elements}) | ftxui::border;
    }));
  }

private:
  ftxui::Component active_;
};

template <class G, class E>
static std::vector<std::shared_ptr<G>> load(const E &elements) {
  return elements | std::views::transform([](const auto &element) {
           return std::make_shared<G>(std::addressof(element));
         }) |
         std::ranges::to<std::vector>();
}

template <class E>
static void add_column(ftxui::Components &columns, const E &elements) {
  if (elements.empty())
    return;

  ftxui::Components column;
  std::ranges::copy(elements, std::back_insert_iterator(column));
  columns.emplace_back(ftxui::Container::Vertical(std::move(column)));
}

export namespace detail {
class tconfiguration final : public ftxui::ComponentBase {
public:
  tconfiguration()
      : labels_(load<tlabel>(data::get_state().labels)),
        projects_(load<tproject>(data::get_state().projects)),
        groups_(load<tgroup>(data::get_state().groups)) {
    add_children();
  }

  ftxui::Element Render() override {
    ftxui::Elements columns;
    std::size_t index = 0;
    if (!labels_.empty())
      columns.emplace_back(
          ftxui::window(ftxui::text("Labels"),
                        ChildAt(0)->ChildAt(index++)->Render()) |
          ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 19) |
          ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 67));
    if (!projects_.empty())
      columns.emplace_back(
          ftxui::window(ftxui::text("Projects"),
                        ChildAt(0)->ChildAt(index++)->Render()) |
          ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 19) |
          ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 67));
    if (!groups_.empty())
      columns.emplace_back(
          ftxui::window(ftxui::text("Groups"),
                        ChildAt(0)->ChildAt(index++)->Render()) |
          ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 19) |
          ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 67));

    return ftxui::hbox(columns);
  }

private:
  void add_children() {
    ftxui::Components columns;
    add_column(columns, labels_);
    add_column(columns, projects_);
    add_column(columns, groups_);
    Add(ftxui::Container::Horizontal({columns}));
  }

  std::vector<std::shared_ptr<tlabel>> labels_;
  std::vector<std::shared_ptr<tproject>> projects_;
  std::vector<std::shared_ptr<tgroup>> groups_;
};

} // namespace detail
