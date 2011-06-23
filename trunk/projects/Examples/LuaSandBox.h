/*-----------------------------------------.---------------------------------.
| Filename: LuaSandBox.h                   | Lua Sand Box                    |
| Author  : Francis Maes                   |                                 |
| Started : 23/06/2011 16:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_LUA_SAND_BOX_H_
# define LBCPP_EXAMPLES_LUA_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>

extern "C" {

# include "../../src/Lua/lua/lua.h"
# include "../../src/Lua/lua/lauxlib.h"
# include "../../src/Lua/lua/lualib.h"

}; /* extern "C" */

namespace lbcpp
{

class LuaSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    const char* code = "print (\"Hello World!\")";

    lua_State* L = lua_open();   /* opens Lua */
    luaL_openlibs(L);
    luaL_loadbuffer(L, code, strlen (code), "myCode");
    lua_pcall(L, 0, 0, 0);

#if 0   
      while (fgets(buff, sizeof(buff), stdin) != NULL) {
        error = luaL_loadbuffer(L, buff, strlen(buff), "line") ||
                lua_pcall(L, 0, 0, 0);
        if (error) {
          fprintf(stderr, "%s", lua_tostring(L, -1));
          lua_pop(L, 1);  /* pop error message from the stack */
        }
      }
#endif // 0
    lua_close(L);
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_FEATURE_GENERATORS_H_
