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

class MyLuaObject : public Object
{
public:
  MyLuaObject() : d(8.6) {}
  double d;
};

typedef ReferenceCountedObjectPtr<MyLuaObject> MyLuaObjectPtr;

extern ClassPtr myLuaObjectClass;

class LuaSandBox : public WorkUnit
{
public:
  static MyLuaObjectPtr& checkObject(lua_State* L)
  {
    void *ud = luaL_checkudata(L, 1, "MyLuaObject");
    luaL_argcheck(L, ud != NULL, 1, "`MyLuaObject' expected");
    return *(MyLuaObjectPtr*)ud;
  }

  static int newObject(lua_State* L)
  {
    Object** a = (Object** )lua_newuserdata(L, sizeof (Object*));
    *a = new MyLuaObject();
    (*a)->incrementReferenceCounter();
    luaL_getmetatable(L, "MyLuaObject");
    lua_setmetatable(L, -2);
    return 1;  /* new userdatum is already on the stack */
  }

  static int objectToString(lua_State* L)
  {
    MyLuaObjectPtr& object = checkObject(L);
    String str = object->toString();
    lua_pushstring(L, str);
    return 1;
  }

  static int objectIndexFunction(lua_State* L)
  {
    int type1 = lua_type(L, 1);
    int type2 = lua_type(L, 2);

    MyLuaObjectPtr& object = checkObject(L);
    if (lua_isstring(L, 2))
    {
      const char* string = luaL_checkstring(L, 2);
      
      if (!strcmp(string, "toto"))
      {
        lua_pushcfunction(L, toto);
        return 1;
      }

      int index = object->getClass()->findMemberVariable(string);
      if (index < 0)
        return 0;

      LuaState(L).pushVariable(object->getVariable(index));
      return 1;
    }
    return 0;
  }

  static int toto(lua_State* L)
  {
    MyLuaObjectPtr& object = checkObject(L);
    lua_pushnumber(L, object->d);
    return 1;
  }

  int luaopen_MyLuaObject(lua_State *L)
  {
    String name = T("MyLuaObject");// myLuaObjectClass
    bool ok = (luaL_newmetatable(L, name) == 1);
    jassert(ok);
    //lua_pushstring(L, "__index");
    //lua_pushvalue(L, -2);  /* pushes the metatable */
    //lua_settable(L, -3);  /* metatable.__index = metatable */
  
    static const struct luaL_reg functions[] = {
      {"new", newObject},
      {NULL, NULL}
    };
    
    static const struct luaL_reg methods[] = {
      {"__index", objectIndexFunction},
      {"__tostring", objectToString},
      {"toto", toto},
      {NULL, NULL}
    };
    
    luaL_openlib(L, NULL, methods, 0);

    luaL_openlib(L, name, functions, 0);
    return 1;
  }

  static int contextError(lua_State* L)
  {
    const char* what = luaL_checkstring(L, 1);
    defaultExecutionContext().warningCallback(what);
    return 0;
  }

  virtual Variable run(ExecutionContext& context)
  {
    LuaStatePtr lua = new LuaState(context);

    luaopen_MyLuaObject(lua->L);
    

    const char* helloWorldCode = "print (\"Hello World!\")\n"
      "a = MyLuaObject.new()\n"
      "print (a.d)\n"
      "print (a:toto())\n"
      "print (a)\n";

    if (!lua->execute(helloWorldCode))
      return false;

    return true;
/*
    lua->pushFunction(blabla);
    lua->setGlobal("context_error");

    const char* code = "context_error(\"coucou\")";
    return lua->execute(code);*/

/*    lua->createTable();
    lua->setTableField("toto", 8.6);
    lua->setGlobal("context");

    lua->pushFunction(contextError);
    lua->setGlobal("context_error");

    const char* code = "context_error( context.toto )";
    return lua->execute(code);*/
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_FEATURE_GENERATORS_H_
