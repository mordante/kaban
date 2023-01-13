#include "state.hpp"

import data;
import stl;

template <std::default_initializable T> class tsingleton {
public:
  tsingleton() = default;
  ~tsingleton() = default;
  tsingleton(const tsingleton &) = delete;
  tsingleton(tsingleton &&) = delete;
  tsingleton &operator=(const tsingleton &) = delete;
  tsingleton &operator=(tsingleton &&) = delete;

#if 0
  [[nodiscard]] std::expected<void, nullptr_t> set(std::unique_ptr<T> &&data) {
    if (!data)
      return std::unexpected(nullptr);
    data_ = std::move(data);
    return {};
  }
#else
  void set(std::unique_ptr<T> &&data) {
    if (!data)
      throw 42;
    data_ = std::move(data);
  }

#endif
  // TODO use deducing this.
  [[nodiscard]] T &get() { return *data_; }
  [[nodiscard]] const T &get() const { return *data_; }

private:
  std::unique_ptr<T> data_{std::make_unique<T>()};
};

static tsingleton<data::tstate> state_singleton;

[[nodiscard]] void *get_state() {
  return std::addressof(state_singleton.get());
}

// TODO use this version, but there are memory issues when doing so.
//[[nodiscard]] std::expected<void, nullptr_t>
// void set_state(std::unique_ptr<data::tstate> &&state) {
//  return state_singleton.set(std::move(state));
void set_state(void *state) {
  state_singleton.set(
      std::unique_ptr<data::tstate>{static_cast<data::tstate *>(state)});
}
