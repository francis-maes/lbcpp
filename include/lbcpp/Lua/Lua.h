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

  void pop(int count = 1);
  void remove(int index);
  void insert(int index);

  int getTop() const;

  LuaType getType(int index) const;

  // Nil
  bool isNil(int index) const;
  void pushNil();

  // Boolean
  bool isBoolean(int index) const;
  bool checkBoolean(int index);
  void pushBoolean(bool value);

  // Integer
  bool isInteger(int index) const;
  int toInteger(int index) const;
  int checkInteger(int index);
  void pushInteger(int value);
  void pushInteger(size_t value);

  // Number
  bool isNumber(int index) const;
  double toNumber(int index) const;
  double checkNumber(int index);
  void pushNumber(double value);

  // String
  bool isString(int index) const;
  String toString(int index);
  const char* checkString(int index);
  void pushString(const char* value);
  void pushString(const String& value);
  const char* pushFString(const char* format, ...);

  // Function
  bool isFunction(int index) const;
  LuaFunction toFunction(int index);
  void pushFunction(LuaFunction function);

  // Object
  ObjectPtr& checkObject(int index, TypePtr expectedType);
  ObjectPtr& checkObject(int index);
  void pushObject(ObjectPtr object);

  // File
  File checkFile(int index);

  // Variable
  Variable checkVariable(int index);
  void pushVariable(const Variable& variable);
  
  bool newMetaTable(const char* name);
  void openLibrary(const char* name, const luaL_Reg* functions, size_t numUpValues = 0);

  ExecutionContext& getContext();

  bool call(int numArguments, int numResults);

  void error(const char* message);
  void error(const String& message);

protected:
  lua_State* L;
  bool owned;

  bool processExecuteError(int error);
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_H_