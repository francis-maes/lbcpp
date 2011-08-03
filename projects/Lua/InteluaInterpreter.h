/*-----------------------------------------.---------------------------------.
| Filename: InteluaInterpreter.h           | Intelua File Interpreter        |
| Author  : Francis Maes                   |                                 |
| Started : 03/08/2011 19:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef INTELUA_INTERPRETER_H_
# define INTELUA_INTERPRETER_H_

# include "Node.h"
# include "Scope.h"

namespace lbcpp
{

class InteluaInterpreter;
typedef ReferenceCountedObjectPtr<InteluaInterpreter> InteluaInterpreterPtr;

class InteluaInterpreter : public Object
{
public:
  InteluaInterpreter(ExecutionContext& context, bool verbose = false);
  InteluaInterpreter() {}
  virtual ~InteluaInterpreter();
  
  bool loadBlock(lua::BlockPtr block, const char* chunkName);
  bool loadBuffer(const char* string, const char* chunkName);
  bool loadFile(const File& file);
  bool executeLoadedChunk(const char* chunkName);

  bool executeBuffer(const char* code, const char* chunkName);
  bool executeFile(const File& file);

  static int loadBuffer(LuaState& state);
  static int loadFile(LuaState& state);

protected:
  bool verbose;
  LuaState translatorState;
  LuaState lua;

  lua::NodePtr parse(const File& file, LuaChunkType type = luaStatementBlock);
  lua::NodePtr parse(const char* code, const char* name, LuaChunkType type = luaStatementBlock);
  void rewrite(lua::BlockPtr& block);
  String prettyPrint(const lua::NodePtr& tree) const;
};

extern ClassPtr inteluaInterpreterClass;

class ExecuteLuaString : public WorkUnit
{
public:
  String code;
  String description;
 
  virtual String toShortString() const
  {
    if (description.isNotEmpty())
      return description;
    int n = code.indexOfChar('\n');
    if (n < 0)
      return code;
    else
      return code.substring(0, n) + T("...");
  }

  virtual Variable run(ExecutionContext& context)
  {
    static bool verbose = true;
    InteluaInterpreter interpreter(context, verbose);
    interpreter.setStaticAllocationFlag();
    return interpreter.executeBuffer(code, toShortString());
  }
};

class ExecuteLuaFile : public WorkUnit
{
public:
  File file;

  virtual Variable run(ExecutionContext& context)
  {
    InteluaInterpreter interpreter(context);
    interpreter.setStaticAllocationFlag();
    return interpreter.executeFile(file);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_INTERPRETER_H_
