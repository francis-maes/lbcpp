/*-----------------------------------------.---------------------------------.
| Filename: lua.h                          | Lua Include File                |
| Author  : Francis Maes                   |                                 |
| Started : 01/08/2011 15:33               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef _INCLUDE_LUA_H_
# define _INCLUDE_LUA_H_

# ifdef USE_LUAJIT

# include "luajit/lua.hpp"

# else
#  ifdef USE_LUACPP

extern "C" {
# include "luacpp/lua.h"
# include "luacpp/lauxlib.h"
# include "luacpp/lualib.h"
# include "luacpp/lpeg.h"
}; /* extern "C" */

#  else
#  error No Lua Implementation
#  endif
# endif

#endif // !_INCLUDE_LUA_H_

