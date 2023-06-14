export module gui:helpers;
import ftxui;
import data;
import std;

// TODO Tune foreground colours further.
export namespace detail {
ftxui::Element create_text(std::string text, data::tcolor color) {
  ftxui::Element result = ftxui::text(text);
  switch (color) {
  case data::tcolor::black:
    return result;
  case data::tcolor::red:
    return result | ftxui::bgcolor(ftxui::Color::Red);
  case data::tcolor::green:
    return result | ftxui::bgcolor(ftxui::Color::Green) |
           ftxui::color(ftxui::Color::Black);
  case data::tcolor::yellow:
    return result | ftxui::bgcolor(ftxui::Color::Yellow) |
           ftxui::color(ftxui::Color::Black);
  case data::tcolor::blue:
    return result | ftxui::bgcolor(ftxui::Color::Blue);
  case data::tcolor::magenta:
    return result | ftxui::bgcolor(ftxui::Color::Magenta);
  case data::tcolor::cyan:
    return result | ftxui::bgcolor(ftxui::Color::Cyan) |
           ftxui::color(ftxui::Color::Black);
  case data::tcolor::light_gray:
    return result | ftxui::bgcolor(ftxui::Color::GrayLight) |
           ftxui::color(ftxui::Color::Black);
  case data::tcolor::dark_gray:
    return result | ftxui::bgcolor(ftxui::Color::GrayDark);
  case data::tcolor::light_red:
    return result | ftxui::bgcolor(ftxui::Color::RedLight);
  case data::tcolor::light_green:
    return result | ftxui::bgcolor(ftxui::Color::GreenLight) |
           ftxui::color(ftxui::Color::Black);
  case data::tcolor::light_yellow:
    return result | ftxui::bgcolor(ftxui::Color::YellowLight) |
           ftxui::color(ftxui::Color::Black);
  case data::tcolor::light_blue:
    return result | ftxui::bgcolor(ftxui::Color::BlueLight) |
           ftxui::color(ftxui::Color::Black);
  case data::tcolor::light_magenta:
    return result | ftxui::bgcolor(ftxui::Color::MagentaLight);
  case data::tcolor::light_cyan:
    return result | ftxui::bgcolor(ftxui::Color::CyanLight);
  case data::tcolor::white:
    return result | ftxui::bgcolor(ftxui::Color::White) |
           ftxui::color(ftxui::Color::Black);
  }
}

ftxui::Element create_label(std::string text, data::tcolor color) {
  return create_text("[" + text + "]", color);
}

ftxui::Component create_title(const data::ttask *task) {
  return ftxui::Renderer([=] {
    ftxui::Elements result;
    result.push_back(ftxui::text(std::format("{:3} ", task->id)));

    if (std::size_t project_id =
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
} // namespace detail
