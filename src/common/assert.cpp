#include <microscopes/common/assert.hpp>
#include <iostream>
using namespace std;
using namespace microscopes::common;

_debug_mode_warning::_debug_mode_warning()
{
#ifdef DEBUG_MODE
  cerr << "notice: debug mode is enabled" << endl;
#endif
}

// static initializer will fire the notice whenever the library is first
// initialized
const _debug_mode_warning
_debug_mode_warning::_Instance;
