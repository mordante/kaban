module;
#include <charconv>
#include <chrono>
#include <string_view>
#include <vector>
export module data;

// TODO DO NOT EXPORT
export std::size_t parse_id(std::string_view input) {
  std::size_t result;
  std::from_chars_result status =
      std::from_chars(input.data(), input.data() + input.size(), result);
  if (status.ec != std::errc{})
    throw 42;

  if (status.ptr != input.data() + input.size())
    throw 42;

  return result;
}

// TODO DO NOT EXPORT
export std::vector<std::size_t> parse_id_list(std::string_view input) {
  std::vector<std::size_t> result;
  if (input.empty())
    return result;
  const char *begin = input.data();
  const char *end = input.data() + input.size();

  while (true) {
    std::size_t value;
    std::from_chars_result status = std::from_chars(begin, end, value);
    if (status.ec != std::errc{})
      throw 42;

    result.push_back(value);
    begin = status.ptr;
    if (begin == end)
      return result;

    if (*begin++ != ',' || *begin++ != ' ')
      throw 42;
  }
}

// TODO DO NOT EXPORT
export bool parse_bool(std::string_view input) {
  if (input == "true")
    return true;

  if (input == "false")
    return false;

  throw 42;
}

// TODO DO NOT EXPORT
export std::chrono::year_month_day parse_date(std::string_view input) {
  std::size_t end = input.find('.');
  if (end == std::string::npos)
    throw 42;

  std::chrono::year year{static_cast<int>(parse_id(input.substr(0, end)))};
  input = input.substr(end + 1);

  end = input.find('.');
  if (end == std::string::npos)
    throw 42;

  std::chrono::month month{
      static_cast<unsigned>(parse_id(input.substr(0, end)))};
  input = input.substr(end + 1);

  std::chrono::day day{static_cast<unsigned>(parse_id(input))};

  return {year, month, day};
}

template <class F> struct delay {
  [[nodiscard]] explicit delay(F &&f) : f_(std::move(f)) {}

  ~delay() { f_(); }

  F f_;
};

export struct parser {
  explicit parser(std::istream &stream)
      : data_(std::istreambuf_iterator<char>(stream), {}) {}
  enum tresult { eof, empty, header, pair };

  std::pair<tresult, std::array<std::string_view, 2>> parse() {

    if (cursor_ == data_.end())
      return {tresult::eof, {}};

    switch (*cursor_) {
    case '\n':
      ++cursor_;
      return {tresult::empty, {}};

    case '[': {
      auto end = std::find(cursor_ + 1, data_.end(), '\n');
      delay _{[&] { cursor_ = end + (end != data_.end()); }};
      return {tresult::header, {std::string_view{cursor_, end}, {}}};
    }
    }

    auto separator = std::find(cursor_ + 1, data_.end(), '=');
    if (separator == data_.end())
      throw 42;

    auto begin = separator + 1;
    auto end = std::find(begin, data_.end(), '\n');

    std::string_view value{begin, end};
    if (value == "<<<") {
      // multiline value
      if (end == data_.end())
        throw 42;
      begin = end + 1;

      while (true) {
        auto pos = end + 1;
        end = std::find(pos, data_.end(), '\n');
        if (end == data_.end())
          throw 42;
        if (std::string_view{pos, end} == ">>>") {
          value = std::string_view{begin, pos - 1};
          break;
        }
      }
    }

    delay _{[&] { cursor_ = end + (end != data_.end()); }};

    return {tresult::pair, {std::string_view{cursor_, separator}, value}};
  }

private:
  std::string data_{};
  std::string::iterator cursor_{data_.begin()};
};
