/*-----------------------------------------.---------------------------------.
| Filename: LuapeProblem.h                 | Lua Program Evolution Problem   |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2011 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_PROBLEM_H_
# define LBCPP_LUAPE_PROBLEM_H_

# include "LuapeGraph.h"

namespace lbcpp
{

class LuapeProblem : public Object
{
public:
  LuapeProblem() : failed(false) {}

  void addInput(const TypePtr& type, const String& name)
    {inputs.push_back(new VariableSignature(type, name));}

  void addFunction(const FunctionPtr& function)
    {functions.push_back(function);}

  size_t getNumFunctions() const
    {return functions.size();}

  FunctionPtr getFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  LuapeGraphPtr createInitialGraph(ExecutionContext& context) const
  {
    LuapeGraphPtr res = new LuapeGraph(true);
    for (size_t i = 0; i < inputs.size(); ++i)
      res->pushNode(context, new LuapeInputNode(inputs[i]->getType(), inputs[i]->getName(), i));
    return res;
  }

  static int input(LuaState& state);
  static int function(LuaState& state);
  //static int objective(LuaState& state);
  
protected:
  friend class LuapeProblemClass;

  std::vector<VariableSignaturePtr> inputs;
  std::vector<FunctionPtr> functions;
  bool failed;
};

extern ClassPtr luapeProblemClass;
typedef ReferenceCountedObjectPtr<LuapeProblem> LuapeProblemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_PROBLEM_H_
