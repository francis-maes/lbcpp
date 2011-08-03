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
  {LuaState state(L); return Object::index(state);}

static int objectNewIndex(lua_State* L)
  {LuaState state(L); return Object::newIndex(state);}

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
      {"__tostring", objectToString},
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
  int oldTop = getTop();

  getGlobal("__errorHandler");
  insert(1);

  // todo: more elaborated error handler
  int res = lua_pcall(L, numArguments, numResults, 1);
  bool ok = processExecuteError(res);
  lua_remove(L, 1);

  int newTop = getTop();

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

bool LuaState::execute(const char* code, const char* codeName, bool verbose)
{
  ExecutionContext& context = getContext();

  if (verbose) context.enterScope(String("Lua parsing ") + codeName); 
  bool ok = processExecuteError(luaL_loadbuffer(L, code, strlen(code), codeName));
  if (verbose) context.leaveScope(ok);
  if (!ok)
    return false;

  if (verbose) context.enterScope(String("Lua interpreting ") + codeName);
  ok = call(0, 0);
  if (verbose) context.leaveScope(ok);
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

void LuaState::pushBoolean(bool value)
  {lua_pushboolean(L, value ? 1 : 0);}

void LuaState::pushString(const char* value)
  {lua_pushstring(L, value);}

void LuaState::pushNumber(double value)
  {lua_pushnumber(L, value);}

void LuaState::pushInteger(size_t value)
  {lua_pushinteger(L, (int)value);}

void LuaState::pushInteger(int value)
  {lua_pushinteger(L, value);}

void LuaState::pushFunction(lua_CFunction function)
  {lua_pushcfunction(L, function);}

void LuaState::pushObject(ObjectPtr object)
{
  ObjectPtr* res = (ObjectPtr* )lua_newuserdata(L, sizeof (ObjectPtr));
  memcpy(res, &object, sizeof (ObjectPtr));
  memset(&object, 0, sizeof (ObjectPtr));
  luaL_getmetatable(L, "LBCppObject");
  lua_setmetatable(L, -2);
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
    pushString(makeString(variable.getString()));
  else if (variable.isObject())
    pushObject(variable.getObject());
  else
    jassert(false);
  // todo: continue ...
}

void LuaState::pop(int count) const
  {lua_pop(L, count);}

int LuaState::getTop() const
  {return lua_gettop(L);}

LuaType LuaState::getType(int index) const
  {return (LuaType)lua_type(L, index);}

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

bool LuaState::isInteger(int index) const
  {return lua_isnumber(L, index) != 0;} // fixme: not distinction between numbers and integers

bool LuaState::isBoolean(int index) const
  {return lua_type(L, index) == LUA_TBOOLEAN;}

bool LuaState::checkBoolean(int index)
  {return lua_toboolean(L, index) != 0;}

double LuaState::checkNumber(int index)
  {return luaL_checknumber(L, index);}

int LuaState::checkInteger(int index)
  {return luaL_checkinteger(L, index);}

const char* LuaState::checkString(int index)
  {return luaL_checkstring(L, index);}

File LuaState::checkFile(int index)
{
  const char* name = checkString(index);
  if (!name)
    return File::nonexistent;
  return getContext().getFile(name);
}

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

int LuaState::returnObject(ObjectPtr object)
{
  if (object)
  {
    pushObject(object);
    return 1;
  }
  else
    return 0;
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

const char* LuaState::makeString(const String& str)
{
  const char* res = lua_pushfstring(L, str); // FIXME: find a better way
  pop();
  return res;
}

void LuaState::error(const char* message)
{
  pushString(message);
  lua_error(L);
}

void LuaState::insert(int index)
  {lua_insert(L, index);}

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
      reg.name = state.makeString(luaFunction->getName());
      reg.func = luaFunction->getFunction();
      functions.push_back(reg);
    }
  }
  luaL_reg reg;
  reg.name = NULL;
  reg.func = NULL;
  functions.push_back(reg);

  state.openLibrary(state.makeString("lbcpp." + name), &functions[0]);
}
