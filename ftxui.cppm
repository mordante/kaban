module;
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

export module ftxui;

export namespace ftxui {

using ftxui::bgcolor;
using ftxui::border;
using ftxui::Button;
using ftxui::Checkbox;
using ftxui::Color;
using ftxui::color;
using ftxui::Component;
using ftxui::ComponentBase;
using ftxui::Components;
using ftxui::Element;
using ftxui::Elements;
using ftxui::Event;
using ftxui::filler;
using ftxui::hbox;
using ftxui::hflow;
using ftxui::Maybe;
using ftxui::Renderer;
using ftxui::Screen;
using ftxui::ScreenInteractive;
using ftxui::size;
using ftxui::text;
using ftxui::Toggle;
using ftxui::vbox;
using ftxui::window;
using ftxui::xflex;
using ftxui::yflex;
using ftxui::operator|;

namespace Container {
using ftxui::Container::Horizontal;
using ftxui::Container::Tab;
using ftxui::Container::Vertical;
} // namespace Container

ftxui::Element multiline_text(const std::string &the_text) {
  ftxui::Elements output;
  std::stringstream ss(the_text);
  std::string line;
  while (std::getline(ss, line)) {
    output.push_back(ftxui::paragraph(line));
  }
  return ftxui::vbox(output);
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
