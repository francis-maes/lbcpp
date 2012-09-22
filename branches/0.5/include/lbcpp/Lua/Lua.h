/*-----------------------------------------.---------------------------------.
| Filename: Lua.h                          | Lua Include File                |
| Author  : Francis Maes                   |                                 |
| Started : 23/06/2011 20:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_H_
# define LBCPP_LUA_H_

# include "../Core/Object.h"
# include "../Core/Container.h"
# include "../Core/Function.h"

namespace lbcpp
{

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
  LuaState(const LuaState& other);
  LuaState(lua_State* L = NULL);
  virtual ~LuaState();

  LuaState& operator=(const LuaState& other);

  LuaState newThread() const;
  LuaState cloneThread() const;

  void clear();
  bool exists() const;

  operator lua_State*()
    {return L;}

  // Code interpretation
  bool loadBuffer(const char* code, const char* chunkName);
  bool execute(const char* code, const char* codeName = "code", bool verbose = false);
  bool execute(const File& luaFile);

  // General stack operations
  int getTop() const;
  void setTop(int size);
  LuaType getType(int index) const;
  void pop(int count = 1);
  void remove(int index);
  void insert(int index);

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
  LuaCFunction toFunction(int index);
  void pushFunction(LuaCFunction function);

  // Table
  bool isTable(int index) const;
  Variable getTableVariable(int index, const char* key);
  void setTableField(const char *name, double value);

  // Object
  ObjectPtr& checkObject(int index, TypePtr expectedType);
  ObjectPtr& checkObject(int index);
  void pushObject(ObjectPtr object);

  // File
  File checkFile(int index);

  // Type
  TypePtr checkType(int index);

  // Variable
  Variable checkVariable(int index);
  void pushVariable(const Variable& variable);

  // References
  int toReference(int index);
  void pushReference(int reference);
  void freeReference(int reference);

  // Global variables
  void setGlobal(const char* name);
  void getGlobal(const char* name);
  void getGlobal(const char* scopeName, const char* name);

  // Function call
  bool call(int numArguments, int numResults);

  // Errors
  void error(const char* message);
  void error(const String& message);
  bool processExecuteError(int error);

  // misc
  bool newMetaTable(const char* name);
  void openLibrary(const char* name, const luaL_Reg* functions, size_t numUpValues = 0);
  size_t length(int index) const;

  // threads & coroutines
  int resume(int numArguments);

  void pushValueFrom(const LuaState& source, int index);


  ExecutionContext& getContext();

  CriticalSection lock;

protected:
  lua_State* L;
  bool owned;

  LuaState(lua_State* L, bool owned)
    : L(L), owned(owned) {}
};

class LuaWrapperValue : public Object
{
public:
  LuaWrapperValue(const LuaState& state, int reference);
  LuaWrapperValue();
  virtual ~LuaWrapperValue();

  int getReference() const;

protected:
  LuaState state;
  int reference;
};

typedef ReferenceCountedObjectPtr<LuaWrapperValue> LuaWrapperValuePtr;

class LuaWrapperVector : public Container
{
public:
  LuaWrapperVector(const LuaState& state, int index);
  LuaWrapperVector();

  virtual size_t getNumElements() const;
  virtual Variable getElement(size_t index) const;
  virtual void setElement(size_t index, const Variable& value);

protected:
  LuaState state;
  int index;
};

class LuaWrapperFunction : public Function
{
public:
  LuaWrapperFunction(const LuaState& state, int functionReference, const std::vector<TypePtr>& inputTypes, TypePtr outputType);
  LuaWrapperFunction() {}

  static int create(LuaState& state);

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  lbcpp_UseDebuggingNewOperator

protected:
  LuaState state;
  int functionReference;
  std::vector<TypePtr> inputTypes;
  TypePtr outputType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_H_
