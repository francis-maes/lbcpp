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
# include "LuapeInference.h"

namespace lbcpp
{

class LuapeProblem : public Object
{
public:
  LuapeProblem()
    : failed(false) {}

  void addInput(const TypePtr& type, const String& name)
    {inputs.push_back(new VariableSignature(type, name));}

  size_t getNumInputs() const
    {return inputs.size();}

  const VariableSignaturePtr& getInput(size_t index) const
    {jassert(index < inputs.size()); return inputs[index];}

  void addFunction(const LuapeFunctionPtr& function)
    {functions.push_back(function);}

  size_t getNumFunctions() const
    {return functions.size();}

  const LuapeFunctionPtr& getFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  LuapeGraphPtr createInitialGraph(ExecutionContext& context) const
  {
    static const size_t maxCacheSize = 10000;
    LuapeGraphPtr res = new LuapeGraph(maxCacheSize);
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
  std::vector<LuapeFunctionPtr> functions;
  bool failed;
};

typedef ReferenceCountedObjectPtr<LuapeProblem> LuapeProblemPtr;
extern ClassPtr luapeProblemClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_PROBLEM_H_
