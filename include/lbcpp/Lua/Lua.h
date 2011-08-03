/*-----------------------------------------.---------------------------------.
| Filename: Lua.h                          | Lua Include File                |
| Author  : Francis Maes                   |                                 |
| Started : 23/06/2011 20:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_H_
# define LBCPP_LUA_H_

# include "../Core/predeclarations.h"

struct lua_State;
typedef struct lua_State lua_State;
typedef int (*lua_CFunction)(lua_State* L);
typedef struct luaL_Reg luaL_Reg;

namespace lbcpp
{

typedef int (*LuaFunction)(lua_State* L);

enum LuaType
{
  luaTypeNone = -1,
  luaTypeNil = 0,
  luaTypeBoolean,
  luaTypeLightUserData,
  luaTypeNumber,
  luaTypeString,
  luaTypeTable,
  luaTypeFunction,
  luaTypeUserData,
  luaTypeThread
};

class LuaState
{
public:
  LuaState(ExecutionContext& context, bool initializeLuaLibraries = true, bool initializeLBCppLibrary = true, bool verbose = false);
  LuaState(lua_State* L = NULL);
  virtual ~LuaState();

  void clear();

  operator lua_State*()
    {return L;}

  bool loadBuffer(const char* code, const char* chunkName);

  bool execute(const char* code, const char* codeName = "code", bool verbose = false);
  bool execute(const File& luaFile);

  void createTable();
  void setTableField(const char *name, double value);

  void setGlobal(const char* name);
  void getGlobal(const char* name);
  void getGlobal(const char* scopeName, const char* name);

  void pushNil();
  void pushBoolean(bool value);
  void pushString(const char* value);
  void pushInteger(int value);
  void pushInteger(size_t value);
  void pushNumber(double value);
  void pushFunction(LuaFunction function);
  void pushVariable(const Variable& variable);
  void pushObject(ObjectPtr object);

  int returnObject(ObjectPtr object);

  void pop(int count = 1) const;

  int getTop() const;

  LuaType getType(int index) const;

  bool isString(int index) const;
  String toString(int index);

  bool isInteger(int index) const;
  bool isBoolean(int index) const;

  bool isFunction(int index) const;
  LuaFunction toFunction(int index);

  bool checkBoolean(int index);
  int checkInteger(int index);
  double checkNumber(int index);
  const char* checkString(int index);
  ObjectPtr& checkObject(int index, TypePtr expectedType);
  ObjectPtr& checkObject(int index);
  File checkFile(int index);
  Variable checkVariable(int index);
  
  bool newMetaTable(const char* name);

  void openLibrary(const char* name, const luaL_Reg* functions, size_t numUpValues = 0);

  const char* makeString(const String& str);

  ExecutionContext& getContext();

  void error(const char* message);

  void insert(int index);
  bool call(int numArguments, int numResults);

protected:
  lua_State* L;
  bool owned;

  bool processExecuteError(int error);
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_H_
