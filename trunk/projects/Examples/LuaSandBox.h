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

namespace lbcpp
{

class MyLuaObject : public Object
{
public:
  MyLuaObject() : d(8.6) {}
  double d;
};

typedef ReferenceCountedObjectPtr<MyLuaObject> MyLuaObjectPtr;

class LuaSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    LuaState luaState(context);

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
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_FEATURE_GENERATORS_H_
