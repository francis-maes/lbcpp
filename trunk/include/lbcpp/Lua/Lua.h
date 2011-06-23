/*-----------------------------------------.---------------------------------.
| Filename: Lua.h                          | Lua Include File                |
| Author  : Francis Maes                   |                                 |
| Started : 23/06/2011 20:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_H_
# define LBCPP_LUA_H_

extern "C" {

# include "../../../src/Lua/lua/lua.h"
# include "../../../src/Lua/lua/lauxlib.h"
# include "../../../src/Lua/lua/lualib.h"

}; /* extern "C" */

namespace lbcpp
{

class LuaState : public Object
{
public:
  LuaState(ExecutionContext& context)
    : context(context), owned(false)
  {
    L = lua_open();
    luaL_openlibs(L);
  }
  LuaState(lua_State* L) : context(defaultExecutionContext()), L(L), owned(false) {}

  virtual ~LuaState()
    {if (owned) lua_close(L);}

  bool execute(const char* code, const char* codeName = "code")
  {
    int error = luaL_loadbuffer(L, code, strlen(code), codeName) || lua_pcall(L, 0, 0, 0);
    if (error)
    {
      context.errorCallback(lua_tostring(L, -1));
      lua_pop(L, 1);  // pop error message from the stack
      return false;
    }
    return true;
  }

 
  void createTable()
  {
    lua_newtable(L);
  }

  void setTableField(const char *name, double value)
    {pushString(name); pushNumber(value); lua_settable(L, -3);}

  void setGlobal(const char* name)
    {lua_setglobal(L, name);}

  void pushString(const char* value)
    {lua_pushstring(L, value);}

  void pushNumber(double value)
    {lua_pushnumber(L, value);}

  void pushFunction(lua_CFunction function)
    {lua_pushcfunction(L, function);}

  const char* checkString(int index)
    {return luaL_checkstring(L, 1);}

  bool newMetaTable(const char* name)
    {return luaL_newmetatable(L, name) == 1;}

  void pushVariable(const Variable& variable)
  {
    if (variable.isDouble())
      lua_pushnumber(L, variable.getDouble());
    else if (variable.isBoolean())
      lua_pushboolean(L, variable.getBoolean() ? 1 : 0);
    else if (variable.isString())
      lua_pushstring(L, variable.getString());
    else
      jassert(false);
    // todo: continue ...
  }

  lua_State* L;
protected:
  ExecutionContext& context;
  bool owned;
};

typedef ReferenceCountedObjectPtr<LuaState> LuaStatePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_H_
