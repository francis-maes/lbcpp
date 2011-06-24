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
# include <lbcpp/Lua/Lua.h>

// tmp
extern "C" {

# include "../../src/Lua/lua/lua.h"
# include "../../src/Lua/lua/lauxlib.h"
# include "../../src/Lua/lua/lualib.h"

}; /* extern "C" */

namespace lbcpp
{

namespace lua
{

  //////////////////

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
/*
  static int newObject(lua_State* L)
  {
    Object** a = (Object** )lua_newuserdata(L, sizeof (Object*));
    *a = new MyLuaObject();
    (*a)->incrementReferenceCounter();
    luaL_getmetatable(L, "MyLuaObject");
    lua_setmetatable(L, -2);
    return 1;  // new userdatum is already on the stack 
  }*/

  static int objectToString(lua_State* L)
  {
    LuaState state(L);
    ObjectPtr object = state.checkObject(1);
    String str = object->toString();
    lua_pushstring(L, str);
    return 1;
  }

  static int objectIndex(lua_State* L)
  {
    LuaState state(L);

    ObjectPtr object = state.checkObject(1);
    if (lua_isstring(L, 2))
    {
      const char* string = state.checkString(2);

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
};


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
  int initializeLua(lua_State *L)
  {
    bool ok = (luaL_newmetatable(L, "Object") == 1);
    jassert(ok);

    // methods
    /*lua_pushcfunction(L, lua::objectIndex);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, lua::objectToString);
    lua_setfield(L, -2, "__tostring");*/
 
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

  static int contextError(lua_State* L)
  {
    const char* what = luaL_checkstring(L, 1);
    defaultExecutionContext().warningCallback(what);
    return 0;
  }

  virtual Variable run(ExecutionContext& context)
  {
    LuaState luaState(context);
    initializeLua(luaState);

    luaState.pushObject(ObjectPtr(&defaultExecutionContext()));
    luaState.setGlobal("context");

    //for (size_t i = 0; i < lbcpp::getNumLibraries(); ++i)
    //  lbcpp::getLibrary(i)->luaRegister(lua->L);


    while (true)
    {
      std::cout << "> " << std::flush;
      char code[1024];
      std::cin.getline(code, 1024);
      if (code)
        luaState.execute(code);
      else
        break;
    }
    return true;
/*
    const char* helloWorldCode = "print (\"Hello World!\")\n"
      "a = MyLuaObject.new()\n"
      "print (a)\n"
      "print (a.d)\n";

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
