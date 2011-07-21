/*-----------------------------------------.---------------------------------.
| Filename: LuaInterpreter.h               | Lua File Interpreter            |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 12:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_INTERPRETER_H_
# define LBCPP_LUA_INTERPRETER_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Lua/Lua.h>

namespace lbcpp
{

class LuaInterpreter : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    LuaState luaState(context);

    if (luaFile == File::nonexistent)
    {
      // interactive lua
      while (true)
      {
        std::cout << "> " << std::flush;
        char code[1024];
        std::cin.getline(code, 1024);
        if (code && strcmp(code, "exit"))
          luaState.execute(code);
        else
          break;
      }
    }
    else
      luaState.execute(luaFile);
    return true;
  }

protected:
  friend class LuaInterpreterClass;

  File luaFile;
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_FEATURE_GENERATORS_H_
