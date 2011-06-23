/*-----------------------------------------.---------------------------------.
| Filename: LuaRegistrationTemplate.h      | Lua Registration Template       |
| Author  : Francis Maes                   |                                 |
| Started : 23/06/2011 19:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

// Usage:
//
// #define LuaClass MyLuaObject
// #define LuaClassName "MyLuaObject"
// #define AbstractLuaClass
// #include <lbcpp/Lua/LuaRegistrationTemplate.hpp>
// #undef AbstractLuaClass
// #undef LuaClassName
// #undef LuaClass

#define LuaClassPtr ReferenceCountedObjectPtr<LuaClass>

  static ObjectPtr& checkObject(lua_State* L)
  {
    void *ud = luaL_checkudata(L, 1, LuaClassName);
    luaL_argcheck(L, ud != NULL, 1, "`" LuaClassName "' expected");
    return *(ObjectPtr*)ud;
  }

#ifndef AbstractLuaClass
  static int newObject(lua_State* L)
  {
    Object** a = (Object** )lua_newuserdata(L, sizeof (LuaClass*));
    *a = new LuaClass();
    (*a)->incrementReferenceCounter();
    luaL_getmetatable(L, LuaClassName);
    lua_setmetatable(L, -2);
    return 1;  /* new userdatum is already on the stack */
  }
#endif // AbstractLuaClass

  static int objectToString(lua_State* L)
  {
    ObjectPtr& object = checkObject(L);
    String str = object->toString();
    lua_pushstring(L, str);
    return 1;
  }

  static int objectIndexFunction(lua_State* L)
  {
    int type1 = lua_type(L, 1);
    int type2 = lua_type(L, 2);

    ObjectPtr& object = checkObject(L);
    if (lua_isstring(L, 2))
    {
      const char* string = luaL_checkstring(L, 2);
      
      /*if (!strcmp(string, "toto"))
      {
        lua_pushcfunction(L, toto);
        return 1;
      }*/

      int index = object->getClass()->findMemberVariable(string);
      if (index < 0)
        return 0;

      LuaState(L).pushVariable(object->getVariable(index));
      return 1;
    }
    return 0;
  }

  virtual int luaRegister(lua_State *L)
  {
    lua_checkstack(L, 1);
    static const char* name = LuaClassName;
    bool ok = (luaL_newmetatable(L, name) == 1);
    jassert(ok);
    //lua_pushstring(L, "__index");
    //lua_pushvalue(L, -2);  /* pushes the metatable */
    //lua_settable(L, -3);  /* metatable.__index = metatable */
  
    static const struct luaL_reg functions[] = {
#ifndef AbstractLuaClass
      {"new", newObject},
#endif
      {NULL, NULL}
    };
    
    lua_pushcfunction(L, objectIndexFunction);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, objectToString);
    lua_setfield(L, -2, "__tostring");
 
    /*static const struct luaL_reg methods[] = {
      {"toto", toto},
      {NULL, NULL}
    };
    luaL_openlib(L, NULL, methods, 0);*/

    luaL_openlib(L, name, functions, 0);
    return 1;
  }

#undef LuaClassPtr 
