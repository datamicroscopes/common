#pragma once

// this necessary evil exists since I can't figure out a way to get cython to
// compile without passing -DNDEBUG to gcc; gdb_debug does not work

#include <microscopes/common/macros.hpp>

#ifdef DEBUG_MODE
#define MICROSCOPES_ASSERT ALWAYS_ASSERT
#else
#define MICROSCOPES_ASSERT assert
#endif

namespace microscopes {
namespace common {

struct _debug_mode_warning {
  _debug_mode_warning();
  static const _debug_mode_warning _Instance;
};

} // namespace common
} // namespace microscopes
