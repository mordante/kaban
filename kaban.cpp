import data;
import ftxui;
import gui;
import stl;

// Quite often the application fails due to changes in modules not being picked
// up properly. Enable this to force a rebuild, if needed.
static const char *generaton = __TIME__;

int main(int argc, const char *argv[]) {
  char *home = std::getenv("HOME");
  std::ifstream file{home + std::string{"/kaban"}};

  std::string input{std::istreambuf_iterator<char>(file), {}};
  std::expected<std::unique_ptr<data::tstate>, data::tparse_error> result =
      data::parse(input);
  if (!result) {
    data::tparse_error error = std::move(result).error();
    std::cerr << std::format(R"(Failed parsing
{}:{}
{}
{}
)",
                             home + std::string{"/kaban"}, error.line_no,
                             error.line, error.message);

    return 1;
  }
  if (!data::set_state(std::move(result).value())) {
    std::cerr << "Failed to store the state\n";
    return 1;
  }

  int tab = 0;
  std::vector<std::string> labels{"Board", "Configuration"};
  ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
  screen.Loop(ftxui::Container::Vertical({
                  ftxui::Button("Quit", screen.ExitLoopClosure()),
                  // There seem to be some issues with the selection in
                  // FTXUI:
                  // - partly https://github.com/ArthurSonzogni/FTXUI/issues/523
                  // - and another not yet investigated issue.
                  ftxui::Toggle(std::addressof(labels), std::addressof(tab)),
                  ftxui::Container::Tab({gui::board(), gui::configuration()},
                                        std::addressof(tab)),
              })             //
              | ftxui::xflex //
              | ftxui::border);
}
