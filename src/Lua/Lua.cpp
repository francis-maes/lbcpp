/*-----------------------------------------.---------------------------------.
| Filename: Lua.cpp                        | Lua C++ Wrapper                 |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2011 18:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Lua/Lua.h>
#include <lbcpp/Core/Variable.h>

extern "C" {
# include "lua/lua.h"
# include "lua/lauxlib.h"
# include "lua/lualib.h"
}; /* extern "C" */

using namespace lbcpp;

LuaState::LuaState(ExecutionContext& context)
  : context(context), owned(false)
{
  L = lua_open();
  luaL_openlibs(L);
}

LuaState::LuaState(lua_State* L)
  : context(defaultExecutionContext()), L(L), owned(false) {}

LuaState::~LuaState()
  {if (owned) lua_close(L);}

bool LuaState::execute(const char* code, const char* codeName)
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

void LuaState::createTable()
{
  lua_newtable(L);
}

void LuaState::setTableField(const char *name, double value)
  {pushString(name); pushNumber(value); lua_settable(L, -3);}

void LuaState::setGlobal(const char* name)
  {lua_setglobal(L, name);}

void LuaState::pushString(const char* value)
  {lua_pushstring(L, value);}

void LuaState::pushNumber(double value)
  {lua_pushnumber(L, value);}

void LuaState::pushFunction(lua_CFunction function)
  {lua_pushcfunction(L, function);}

bool LuaState::newMetaTable(const char* name)
  {return luaL_newmetatable(L, name) == 1;}

void LuaState::pushVariable(const Variable& variable)
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

const char* LuaState::checkString(int index)
  {return luaL_checkstring(L, index);}

ObjectPtr LuaState::checkObject(int index, TypePtr expectedType)
{
  ExecutionContext& context = defaultExecutionContext();

  ObjectPtr* p = (ObjectPtr* )luaL_checkudata(L, index, "Object");
  if (p)
  {
    TypePtr type = (*p)->getClass();
    if (!type->inheritsFrom(expectedType))
    {
      luaL_error(L, "%s does not inherit from %s", (const char* )type->getName(), (const char* )expectedType->getName());
      return ObjectPtr();
    }
    return *p;
  }

  // value is not a userdata or does not have meta-table
  luaL_error(L, "expected '%s'", (const char* )expectedType->getName());
  return ObjectPtr();
}

ObjectPtr LuaState::checkObject(int index)
  {return checkObject(index, objectClass);}

void LuaState::pushObject(ObjectPtr object)
{
  ObjectPtr* res = (ObjectPtr* )lua_newuserdata(L, sizeof (ObjectPtr));
  memcpy(res, &object, sizeof (ObjectPtr));
  memset(&object, 0, sizeof (ObjectPtr));
  luaL_getmetatable(L, "Object");
  lua_setmetatable(L, -2);
}
