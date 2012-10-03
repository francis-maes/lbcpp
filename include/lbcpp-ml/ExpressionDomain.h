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
# include <lbcpp/Luape/LuapeCache.h>

namespace lbcpp
{

class ExpressionDomain : public Domain
{
public:
  ExpressionDomain(ExpressionUniversePtr universe = ExpressionUniversePtr())
    : universe(universe)
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

  const VariableExpressionPtr& getDomainVariable(size_t index) const
    {jassert(index < variables.size()); return variables[index];}
  
  const std::vector<VariableExpressionPtr>& getVariables() const
    {return variables;}

  VariableExpressionPtr addVariable(const TypePtr& type, const String& name)
    {size_t index = variables.size(); VariableExpressionPtr res(new VariableExpression(type, name, index)); variables.push_back(res); return res;}

  /*
  ** Supervision variable
  */
  VariableExpressionPtr getSupervision() const
    {return supervision;}

  VariableExpressionPtr createSupervision(const TypePtr& type, const String& name = "supervision")
    {jassert(!supervision); supervision = new VariableExpression(type, name, variables.size()); return supervision;}

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

  /*
  ** Cache
  */
  LuapeSamplesCachePtr createCache(size_t numSamples, size_t maxCacheSizeInMb = 512) const
    {return new LuapeSamplesCache(universe, variables, numSamples, maxCacheSizeInMb);}

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
