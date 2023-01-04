#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include <expected>
#include <iostream>

#include <sstream> // multiline

import data;
import gui;
import stl;

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

int main(int argc, const char *argv[]) {
  char *home = std::getenv("HOME");
  std::ifstream file{home + std::string{"/kaban"}};

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
  // data::set_state(std::unique_ptr<data::tstate>{result.value()});
  data::set_state(result.value());

#if 0
  // Part of #43
  int tab = 0;
  std::vector<std::string> labels{"Board", "Configuration"};
#endif
  ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
  screen.Loop(ftxui::Container::Vertical({
                ftxui::Button("Quit", screen.ExitLoopClosure()),
#if 1
                    gui::board()
#else
                    // Selection seems odd in ftxui
                    ftxui::Toggle(std::addressof(labels), std::addressof(tab)),
                    ftxui::Container::Tab({gui::board(), gui::configuration()},
                                          std::addressof(tab)),
#endif
              })             //
              | ftxui::xflex //
              | ftxui::border);
}
