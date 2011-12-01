/*-----------------------------------------.---------------------------------.
| Filename: LuapeProblem.cpp               | Lua Program Evolution Problem   |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 19:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#if 0
#include "LuapeProblem.h"
#include <lbcpp/Lua/Lua.h>
using namespace lbcpp;

int LuapeProblem::input(LuaState& state)
{
  LuapeProblemPtr problem = state.checkObject(1, luapeProblemClass).staticCast<LuapeProblem>();
  String name = state.checkString(2);
  TypePtr type = state.checkType(3);
  problem->inputs.push_back(new VariableSignature(type, name));
  return 0;
}

int LuapeProblem::function(LuaState& state)
{
  // broken

  jassert(false);
  return 0;
#if 0
  LuapeProblemPtr problem = state.checkObject(1, luapeProblemClass).staticCast<LuapeProblem>();

  LuapeFunctionPtr function;
  Variable variable = state.checkVariable(2);
  if (variable.getType() == functionClass)
    function = variable.getObjectAndCast<Function>();
  else
  {
    String name = state.checkString(2);
    int functionReference = state.toReference(3);
    int n = state.getTop();
    std::vector<TypePtr> inputTypes;

    for (int i = 4; i < n; ++i)
    {
      inputTypes.push_back(state.checkType(i));
      if (!inputTypes.back())
        return 0;
    }
    TypePtr outputType = state.checkType(n);
    if (!outputType)
      return 0;
   
    function = new LuaWrapperFunction(state, functionReference, inputTypes, outputType);
  }
  problem->functions.push_back(function);
#endif // 0
  return 0;
}

#endif // 0
