export module gui;
import :configuration;
import :board;

import ftxui;
import stl;

export namespace gui {

ftxui::Component board() { return std::make_shared<detail::tboard>(); }
ftxui::Component configuration() {
  return std::make_shared<detail::tconfiguration>();
}

} // namespace gui
