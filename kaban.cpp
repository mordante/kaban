#include <expected>

import data;
import ftxui;
import gui;
import stl;

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
