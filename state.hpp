#ifndef STATE_HPP
#define STATE_HPP

// When including this implementation in data.cppm
// it seemed several compiler optimizations broke this code by
// omitting calls to set_state, and let get_state return a default constructed
// state.
//
// This is a horrible hack and a bug should be reported when a proper
// reproducer can be found. It may be related to
//   https://github.com/llvm/llvm-project/issues/59765
// since there it seems ownership information is wrong, which may lead to unsafe
// optimizations.

[[nodiscard]] void *get_state();

void set_state(void *state);

#endif // STATE_HPP
