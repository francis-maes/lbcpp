/*-----------------------------------------.---------------------------------.
| Filename: SymbRegSandBox.h               | Symbolic Regression SandBox     |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 13:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_SYMB_REG_SANDBOX_H_
# define LBCPP_MOO_SYMB_REG_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "MOOCore.h"

namespace lbcpp
{
#if 0
class ExpressionDomain : public MOODomain
{
public:
  ExpressionDomain(ExpressionUniversePtr universe, size_t maxNumSymbols)
    : universe(universe), maxNumSymbols(maxNumSymbols)
  {
    if (!universe)
      this->universe = new ExpressionUniverse();
  }
  
  const ExpressionUniversePtr& getUniverse() const
    {return universe;}

  /*
  ** Inputs
  */
  size_t getNumInputs() const
    {return inputs.size();}

  const VariableExpressionPtr& getInput(size_t index) const
    {jassert(index < inputs.size()); return inputs[index];}
  
  const std::vector<VariableExpressionPtr>& getInputs() const
    {return inputs;}

  void addInput(const TypePtr& type, const String& name)
    {size_t index = inputs.size(); inputs.push_back(new VariableExpression(type, name, index));}

  /*
  ** Supervision variable
  */
  VariableExpressionPtr getSupervision() const
    {return supervision;}

  /*
  ** Available Functions
  */
  size_t getNumFunctions() const
    {return functions.size();}

  const LuapeFunctionPtr& getFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  void addFunction(const LuapeFunctionPtr& function)
    {functions.push_back(function);}

  /*
  ** Available Constants
  */
  size_t getNumConstants() const
    {return constants.size();}

  const ConstantExpressionPtr& getConstant(size_t index) const
    {jassert(index < constants.size()); return constants[index];}

  void addConstant(const Variable& value)
    {constants.push_back(new ConstantExpression(value));}

  /*
  ** Accepted target types
  */
  bool isTargetTypeAccepted(TypePtr type)
  {
    for (std::set<TypePtr>::const_iterator it = targetTypes.begin(); it != targetTypes.end(); ++it)
      if (type->inheritsFrom(*it))
        return true;
    return false;
  }

  void addTargetType(TypePtr type)
    {targetTypes.insert(type);}
  void clearTargetTypes()
    {targetTypes.clear();}

  /*
  ** Type search space
  */
  LuapeGraphBuilderTypeSearchSpacePtr getTypeSearchSpace() const
  {
    /*if (!typeSearchSpace)
    {
      LuapeGraphBuilderTypeSearchSpacePtr res = new LuapeGraphBuilderTypeSearchSpace(refCountedPointerFromThis(this), initialState, complexity);
      res->pruneStates(context, verbose);
      res->assignStateIndices(context);
      const_cast<ExpressionDomain* >(res)->typeSearchSpace = typeSearchSpace;
    }*/
    return typeSearchSpace;
  }

protected:
  ExpressionUniversePtr universe;
  size_t maxNumSymbols;
  std::vector<VariableExpressionPtr> inputs;
  VariableExpressionPtr supervision;
  std::vector<ConstantExpressionPtr> constants;
  std::vector<LuapeFunctionPtr> functions;
  std::set<TypePtr> targetTypes;
  LuapeSamplesCachePtr trainingCache;
  LuapeSamplesCachePtr validationCache;

  LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;
};
#endif // 0

class SymbRegSandBox : public WorkUnit
{
public:
  SymbRegSandBox() : numEvaluations(1000), verbosity(1) {}

  virtual Variable run(ExecutionContext& context)
  {
    return true;
  }

protected:
  friend class SymbRegSandBoxClass;

  size_t numEvaluations;
  size_t verbosity;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SYMB_REG_SANDBOX_H_
