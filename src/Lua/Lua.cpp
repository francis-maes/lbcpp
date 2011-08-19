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
#include <lbcpp/Core/Library.h>
#include <lbcpp/library.h>
#include "../../lua/lua.h"
using namespace lbcpp;

static int objectIndex(lua_State* L)
  {LuaState state(L); ObjectPtr object = state.checkObject(1); state.remove(1); return object->__index(state);}

static int objectNewIndex(lua_State* L)
  {LuaState state(L); ObjectPtr object = state.checkObject(1); state.remove(1); return object->__newIndex(state);}

static int objectLen(lua_State* L)
  {LuaState state(L); ObjectPtr object = state.checkObject(1); state.remove(1); return object->__len(state);}

static int objectAdd(lua_State* L)
  {LuaState state(L); ObjectPtr object = state.checkObject(1); state.remove(1); return object->__add(state);}

static int objectSub(lua_State* L)
  {LuaState state(L); ObjectPtr object = state.checkObject(1); state.remove(1); return object->__sub(state);}

static int objectMul(lua_State* L)
  {LuaState state(L); ObjectPtr object = state.checkObject(1); state.remove(1); return object->__mul(state);}

static int objectDiv(lua_State* L)
  {LuaState state(L); ObjectPtr object = state.checkObject(1); state.remove(1); return object->__div(state);}

static int objectEq(lua_State* L)
  {LuaState state(L); ObjectPtr object = state.checkObject(1); state.remove(1); return object->__eq(state);}

static int objectToString(lua_State* L)
  {LuaState state(L); return Object::toShortString(state);}

static int objectGarbageCollect(lua_State* L)
  {LuaState state(L); return Object::garbageCollect(state);}

LuaState::LuaState(ExecutionContext& context, bool initializeLuaLibraries, bool initializeLBCppLibrary, bool verbose)
  : owned(true)
{
  if (verbose) context.enterScope("Lua Open");
  L = lua_open();
  if (verbose) context.leaveScope(true);
  if (initializeLuaLibraries)
  {
    if (verbose) context.enterScope("Initialize Lua Libraries");
    luaL_openlibs(L);
    if (verbose) context.leaveScope(true);
    //int n = luaopen_lpeg(L);
    //pop(n);
  }
  if (initializeLBCppLibrary)
  {
    if (verbose) context.enterScope("Initialize Lbcpp Library");
    luaL_newmetatable(L, "LBCppObject");
    static const struct luaL_reg methods[] = {
      {"__index", objectIndex},
      {"__newindex", objectNewIndex},
      {"__len", objectLen},
      {"__tostring", objectToString},
      {"__add", objectAdd},
      {"__sub", objectSub},
      {"__mul", objectMul},
      {"__div", objectDiv},
      {"__eq", objectEq},
      {"__gc", objectGarbageCollect},
      {NULL, NULL}
    };
    luaL_openlib(L, NULL, methods, 0);

    for (size_t i = 0; i < lbcpp::getNumLibraries(); ++i)
      lbcpp::getLibrary(i)->luaRegister(*this);
    
    pop(1);

    pushObject(ObjectPtr(&context));
    setGlobal("context");

    if (verbose) context.leaveScope(true);
  }
}

LuaState::LuaState(lua_State* L)
  : L(L), owned(false) {}

LuaState::~LuaState()
{
  clear();
}

void LuaState::clear()
{
  if (owned)
  {
    owned = false;
    lua_close(L);
  }
  L = NULL;
}

bool LuaState::call(int numArguments, int numResults)
{
  getGlobal("__errorHandler");
  insert(1);
  int res = lua_pcall(L, numArguments, numResults, 1);
  bool ok = processExecuteError(res);
  lua_remove(L, 1);
  return ok;
}

bool LuaState::processExecuteError(int error)
{
  if (error)
  {
    String what = toString(-1);
    pop(1); // pop error message from the stack

    ExecutionContext& context = getContext();
    if (error == LUA_ERRRUN)
      context.errorCallback(what.isEmpty() ? "Runtime error" : what);
    else if (error == LUA_ERRMEM)
      context.errorCallback("Memory allocation error: " + what);
    else if (error == LUA_ERRERR)
      context.errorCallback("Error while running the error handler function: " + what);
    else
      context.errorCallback("Unknown error: " + what);
    return false;
  }
  return true;
}

bool LuaState::loadBuffer(const char* code, const char* chunkName)
{
  // if ok, the loaded chunk is on returned on the stack, otherwise nothing is left on the stack
  return processExecuteError(luaL_loadbuffer(L, code, strlen(code), chunkName));
}

bool LuaState::execute(const char* code, const char* chunkName, bool verbose)
{
  ExecutionContext& context = getContext();

  if (verbose) context.enterScope(String("Lua loading ") + chunkName); 
  bool ok = loadBuffer(code, chunkName);
  if (verbose) context.leaveScope(ok);
  if (ok)
  {
    if (verbose) context.enterScope(String("Lua interpreting ") + chunkName);
    ok = call(0, 0);
    if (verbose) context.leaveScope(ok);
  }
  return ok;
}

bool LuaState::execute(const File& luaFile)
{
  return processExecuteError(luaL_loadfile(L, luaFile.getFullPathName())) && call(0, 0);
}

void LuaState::setGlobal(const char* name)
  {lua_setglobal(L, name);}

void LuaState::getGlobal(const char* name)
  {lua_getglobal(L, name);}

void LuaState::getGlobal(const char* scopeName, const char* name)
{
  getGlobal(scopeName);
  if (!lua_isnil(L, -1))
  {
    lua_getfield(L, -1, name);
    lua_remove(L, -2); // remove scope from stack
  }
}

/*
** Nil
*/
bool LuaState::isNil(int index) const
  {return lua_isnil(L, index) != 0;}

void LuaState::pushNil()
  {lua_pushnil(L);}

/*
** Boolean
*/
bool LuaState::isBoolean(int index) const
  {return lua_type(L, index) == LUA_TBOOLEAN;}

bool LuaState::checkBoolean(int index)
  {return lua_toboolean(L, index) != 0;}

void LuaState::pushBoolean(bool value)
  {lua_pushboolean(L, value ? 1 : 0);}

/*
** Integer
*/
bool LuaState::isInteger(int index) const
  {return lua_isnumber(L, index) != 0;} // fixme: no distinction between numbers and integers

int LuaState::toInteger(int index) const
  {return lua_tointeger(L, index);}

int LuaState::checkInteger(int index)
  {return luaL_checkinteger(L, index);}

void LuaState::pushInteger(size_t value)
  {lua_pushinteger(L, (int)value);}

void LuaState::pushInteger(int value)
  {lua_pushinteger(L, value);}

/*
** Number
*/
bool LuaState::isNumber(int index) const
  {return lua_isnumber(L, index) != 0;}

double LuaState::toNumber(int index) const
  {return lua_tonumber(L, index);}

double LuaState::checkNumber(int index)
  {return luaL_checknumber(L, index);}

void LuaState::pushNumber(double value)
  {lua_pushnumber(L, value);}

/*
** String
*/
bool LuaState::isString(int index) const
  {return lua_isstring(L, index) != 0;}

String LuaState::toString(int index)
{
  // returns tostring(value)
  getGlobal("tostring");
  lua_pushvalue(L, index);
  lua_call(L, 1, 1);
  String res = checkString(-1);
  pop(1);
  return res;

  /*Variable v = checkVariable(index);
  if (v.isString())
    return v.getString();
  return v.isObject() ? v.toShortString() : v.toString();*/
}

const char* LuaState::checkString(int index)
  {return luaL_checkstring(L, index);}

void LuaState::pushString(const char* value)
  {lua_pushstring(L, value);}

const char* LuaState::pushFString(const char* format, ...)
{
  va_list argp;
  va_start(argp, format);
  const char* res = lua_pushfstring(L, format, argp);
  va_end(argp);
  return res;
}

void LuaState::pushString(const String& value)
  {pushFString(value);}

/*
** Function
*/
bool LuaState::isFunction(int index) const
  {return lua_iscfunction(L, index) != 0;}

LuaFunction LuaState::toFunction(int index)
  {return lua_tocfunction(L, index);}

void LuaState::pushFunction(lua_CFunction function)
  {lua_pushcfunction(L, function);}

/*
** Object
*/
ObjectPtr& LuaState::checkObject(int index, TypePtr expectedType)
{
  ObjectPtr* p = (ObjectPtr* )luaL_checkudata(L, index, "LBCppObject");
  if (p)
  {
    TypePtr type = (*p)->getClass();
    if (type->inheritsFrom(expectedType))
      return *p;
    else
      luaL_error(L, "%s does not inherit from %s", (const char* )type->getName(), (const char* )expectedType->getName());
  }
  else
    // value is not a userdata or does not have meta-table
    luaL_error(L, "expected '%s'", (const char* )expectedType->getName());

  return *(ObjectPtr* )0;
}

ObjectPtr& LuaState::checkObject(int index)
  {return checkObject(index, objectClass);}

void LuaState::pushObject(ObjectPtr object)
{
  ObjectPtr* res = (ObjectPtr* )lua_newuserdata(L, sizeof (ObjectPtr));
  memcpy(res, &object, sizeof (ObjectPtr));
  memset(&object, 0, sizeof (ObjectPtr));
  luaL_getmetatable(L, "LBCppObject");
  lua_setmetatable(L, -2);
}

/*
** Variable
*/
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

void LuaState::pushVariable(const Variable& variable)
{
  if (variable.isDouble())
    pushNumber(variable.getDouble());
  else if (variable.isBoolean())
    pushBoolean(variable.getBoolean());
  else if (variable.isInteger())
    pushInteger(variable.getInteger());
  else if (variable.isString())
    pushString(variable.getString());
  else if (variable.isObject())
    pushObject(variable.getObject());
  else
    jassert(false);
  // todo: continue ...
}

void LuaState::pop(int count)
  {lua_pop(L, count);}

void LuaState::remove(int index)
  {lua_remove(L, index);}

int LuaState::getTop() const
  {return lua_gettop(L);}

LuaType LuaState::getType(int index) const
  {return (LuaType)lua_type(L, index);}

File LuaState::checkFile(int index)
{
  const char* name = checkString(index);
  if (!name)
    return File::nonexistent;
  return getContext().getFile(name);
}

void LuaState::createTable()
  {lua_newtable(L);}

void LuaState::setTableField(const char *name, double value)
  {pushString(name); pushNumber(value); lua_settable(L, -3);}

bool LuaState::newMetaTable(const char* name)
  {return luaL_newmetatable(L, name) == 1;}

void LuaState::openLibrary(const char* name, const luaL_Reg* functions, size_t numUpValues)
{
  luaL_openlib(L, name, functions, (int)numUpValues);
  pop(1);
}

ExecutionContext& LuaState::getContext()
{
  getGlobal("context");
  ExecutionContextPtr res = checkObject(-1, executionContextClass).staticCast<ExecutionContext>();
  pop(1);
  return *res;
}

void LuaState::error(const char* message)
{
  error(String(message));
}

void LuaState::error(const String& message)
{
  luaL_where(L, 1);
  pushString(message);
  lua_concat(L, 2);
  lua_error(L);
}
  
void LuaState::insert(int index)
{
  lua_insert(L, index);
}

void Type::luaRegister(LuaState& state) const
{
  std::vector<luaL_Reg> functions;
  size_t n = getNumMemberFunctions();
  for (size_t i = 0; i < n; ++i)
  {
    LuaFunctionSignaturePtr luaFunction = getMemberFunction(i).dynamicCast<LuaFunctionSignature>();
    if (luaFunction)// && luaFunction->isStatic())
    {
      luaL_reg reg;
      state.pushString(luaFunction->getName());
      reg.name = state.checkString(-1);
      state.pop(1);
      //reg.name = luaFunction->getName();
      reg.func = luaFunction->getFunction();
      functions.push_back(reg);
    }
  }
  luaL_reg reg;
  reg.name = NULL;
  reg.func = NULL;
  functions.push_back(reg);

  String libraryName = "lbcpp." + name;
  state.openLibrary(libraryName, &functions[0]);
}
