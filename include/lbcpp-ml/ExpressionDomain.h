/*-----------------------------------------.---------------------------------.
| Filename: ExpressionDomain.h             | Expression Domain               |
| Author  : Francis Maes                   |                                 |
| Started : 03/10/2012 13:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_DOMAIN_H_
# define LBCPP_ML_EXPRESSION_DOMAIN_H_

# include "Domain.h"
# include "Expression.h"
# include "ExpressionUniverse.h"

namespace lbcpp
{

class ExpressionDomain : public Domain
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
  ** Variables
  */
  size_t getNumVariables() const
    {return variables.size();}

  const VariableExpressionPtr& getVariable(size_t index) const
    {jassert(index < variables.size()); return variables[index];}
  
  const std::vector<VariableExpressionPtr>& getVariables() const
    {return variables;}

  void addVariable(const TypePtr& type, const String& name)
    {size_t index = variables.size(); variables.push_back(new VariableExpression(type, name, index));}

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

  const FunctionPtr& getFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  void addFunction(const FunctionPtr& function)
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

#if 0
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
#endif // 0

protected:
  ExpressionUniversePtr universe;
  size_t maxNumSymbols;
  std::vector<VariableExpressionPtr> variables;
  VariableExpressionPtr supervision;
  std::vector<ConstantExpressionPtr> constants;
  std::vector<FunctionPtr> functions;
  std::set<TypePtr> targetTypes;
  LuapeSamplesCachePtr trainingCache;
  LuapeSamplesCachePtr validationCache;

  //LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_DOMAIN_H_
