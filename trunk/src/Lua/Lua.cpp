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

namespace lua
{

  static int createObject(lua_State* L)
  {
    LuaState state(L);
    const char* className = state.checkString(1);
    TypePtr type = getType(className);
    if (!type)
      return 0;
    ObjectPtr res = Object::create(type);
    if (!res)
      return 0;
    state.pushObject(res);
    return 1;
  }

  static int objectToString(lua_State* L)
  {
    LuaState state(L);
    ObjectPtr object = state.checkObject(1);
    state.pushString(object->toString());
    return 1;
  }

  static int objectIndex(lua_State* L)
  {
    LuaState state(L);

    ObjectPtr object = state.checkObject(1);
    if (state.isString(2))
    {
      String string = state.checkString(2);

      TypePtr type = object->getClass();
      int index = type->findMemberVariable(string);
      if (index >= 0)
      {
        state.pushVariable(object->getVariable(index));
        return 1;
      }

      index = type->findMemberFunction(string);
      if (index >= 0)
      {
        LuaFunctionSignaturePtr signature = type->getMemberFunction(index).dynamicCast<LuaFunctionSignature>();
        if (!signature)
          return 0;

        state.pushFunction(signature->getFunction());
        return 1;
      }
    }
    return 0;
  }

  int initializeObject(lua_State* L)
  {
    if (!luaL_newmetatable(L, "Object"))
      return 0;

    // methods
    static const struct luaL_reg methods[] = {
      {"__index", lua::objectIndex},
      {"__tostring", lua::objectToString},
    //  {"toto", toto},
      {NULL, NULL}
    };
    luaL_openlib(L, NULL, methods, 0);

    // functions
    static const struct luaL_reg functions[] = {
      {"create", lua::createObject},
      {NULL, NULL}
    };
    luaL_openlib(L, "Object", functions, 0);
    return 1;
  }

}; /* namespace lua */

LuaState::LuaState(ExecutionContext& context, bool initializeLuaLibraries, bool initializeLBCppLibrary)
  : context(context), owned(false)
{
  L = lua_open();
  if (initializeLuaLibraries)
    luaL_openlibs(L);
  if (initializeLBCppLibrary)
  {
    lua::initializeObject(L);
    pushObject(ObjectPtr(&context));
    setGlobal("context");
  }
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

int LuaState::getTop() const
  {return lua_gettop(L);}

LuaType LuaState::getType(int index) const
  {return (LuaType)lua_type(L, index);}

bool LuaState::isString(int index) const
  {return lua_isstring(L, index) != 0;}

bool LuaState::checkBoolean(int index)
{
  jassert(false); // not implemented
  return false;
}

double LuaState::checkNumber(int index)
  {return luaL_checknumber(L, index);}

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

Variable LuaState::checkVariable(int index)
{
  LuaType luaType = getType(index);
  switch (luaType)
  {
  case luaTypeNone: 
  case luaTypeNil:      return Variable();
  case luaTypeBoolean:  return Variable(checkBoolean(index));
  case luaTypeNumber:   return Variable(checkNumber(index));
  case luaTypeString:   return Variable(String(checkString(index)));
  case luaTypeUserData: return Variable(checkObject(index));
  case luaTypeLightUserData:
  case luaTypeTable:
  case luaTypeFunction:
  case luaTypeThread:
  default:
    jassert(false); // not implemented
    return Variable();
  }
}

void LuaState::pushObject(ObjectPtr object)
{
  ObjectPtr* res = (ObjectPtr* )lua_newuserdata(L, sizeof (ObjectPtr));
  memcpy(res, &object, sizeof (ObjectPtr));
  memset(&object, 0, sizeof (ObjectPtr));
  luaL_getmetatable(L, "Object");
  lua_setmetatable(L, -2);
}
