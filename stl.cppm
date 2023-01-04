module;

// Partial export of c++23's std module
// Note this is incompete and it's more like a glorified PCH.

// issues with
// - algorithm
// - ranges
// - expected (https://github.com/llvm/llvm-project/issues/59719)

#include <algorithm>
#include <charconv>
#include <chrono>
#include <concepts>
#include <format>
#include <fstream>
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

export module stl;

export {
  using ::operator new;
  using ::operator new[];
  using ::operator delete;
  using ::operator delete[];
} // export

export namespace std {
using std::accumulate;
using std::addressof;
using std::allocate_shared;
using std::allocator_traits;
using std::array;
using std::construct_at;
using std::default_initializable;
using std::errc;
using std::find;
using std::format;
using std::forward;
using std::from_chars;
using std::from_chars_result;
using std::function;
using std::get;
using std::ifstream;
using std::in_place;
using std::indirect_binary_predicate;
using std::istreambuf_iterator;
using std::make_shared;
using std::make_unique;
using std::move;
using std::optional;
using std::pair;
using std::projected;
using std::shared_ptr;
using std::size_t;
using std::span;
using std::string;
using std::string_view;
using std::unique_ptr;
using std::variant;
using std::vector;
using std::operator==;
using std::identity;
namespace chrono {
using std::chrono::day;
using std::chrono::month;
using std::chrono::operator!=;
using std::chrono::operator<;
using std::chrono::operator<=;
using std::chrono::operator<=>;
using std::chrono::operator==;
using std::chrono::operator>;
using std::chrono::operator>=;
using std::chrono::sys_days;
using std::chrono::system_clock;
using std::chrono::year;
using std::chrono::year_month_day;
} // namespace chrono
namespace ranges {
using std::ranges::begin;
using std::ranges::borrowed_iterator_t;
using std::ranges::end;
using std::ranges::equal_to;
using std::ranges::find;
using std::ranges::iterator_t;
} // namespace ranges
} // namespace std
