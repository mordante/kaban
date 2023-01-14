module;
#include <algorithm>

export module gui:configuration;
import :helpers;

import ftxui;
import data;
import stl;

export namespace detail {
class tlabel final : public ftxui::ComponentBase {
public:
  explicit tlabel(const data::tlabel *label) {
    Add(ftxui::Renderer([=] {
      ftxui::Elements elements;
      elements.emplace_back(ftxui::text(std::format("{:3} ", label->id)));
      elements.emplace_back(create_text(label->name, label->color));
      if (!label->description.empty())
        elements.emplace_back(ftxui::text(label->description) | ftxui::border);
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

} // namespace detail
