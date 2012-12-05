/*-----------------------------------------.---------------------------------.
| Filename: ExpressionActionDomainsCache.h | A cache for expression building |
| Author  : Francis Maes                   | actions                         |
| Started : 12/10/2012 12:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_ACTION_DOMAINS_CACHE_H_
# define LBCPP_ML_EXPRESSION_ACTION_DOMAINS_CACHE_H_

# include <ml/ExpressionDomain.h>

namespace lbcpp
{

class ExpressionActionDomainsCache : public Object
{
public:
  ExpressionActionDomainsCache(ExpressionDomainPtr domain)
  {
    // arity-0 actions: constants and variables
    DiscreteDomainPtr actions = new DiscreteDomain();
    for (size_t i = 0; i < domain->getNumConstants(); ++i)
      actions->addElement(domain->getConstant(i));
    for (size_t i = 0; i < domain->getNumInputs(); ++i)
      actions->addElement(domain->getInput(i));
    const std::set<ExpressionPtr>& activeVariables = domain->getActiveVariables();
    for (std::set<ExpressionPtr>::const_iterator it = activeVariables.begin(); it != activeVariables.end(); ++it)
      actions->addElement(*it);
    jassert(actions->getNumElements());
    actionSubsets.push_back(actions);

    // arity-n actions: functions
    size_t maxFunctionArity = domain->getMaxFunctionArity();
    actionSubsets.resize(maxFunctionArity + 2);
    for (size_t arity = 1; arity <= maxFunctionArity; ++arity)
    {
      DiscreteDomainPtr actions = new DiscreteDomain();
      for (size_t i = 0; i < domain->getNumFunctions(); ++i)
      {
        FunctionPtr function = domain->getFunction(i);
        jassert(function->getNumVariables() == 0); // parameterized functions are not supported yet
        if (function->getNumInputs() == arity)
          actions->addElement(function);
      }
      actionSubsets[arity] = actions;
    }
    
    // yield action
    actions = new DiscreteDomain();
    actions->addElement(ObjectPtr());
    actionSubsets.back() = actions;
  }
  ExpressionActionDomainsCache() {}

  DiscreteDomainPtr getActionsByArity(size_t arity) const
    {jassert(arity < actionSubsets.size()); return actionSubsets[arity];}

  DiscreteDomainPtr getActions(const std::vector<size_t>& subsetIndices)
  {
    DiscreteDomainPtr& res = actionSets[subsetIndices];
    if (!res)
    {
      res = new DiscreteDomain();
      for (size_t i = 0; i < subsetIndices.size(); ++i)
      {
        size_t index = subsetIndices[i];
        jassert(index < actionSubsets.size());
        res->addElements(actionSubsets[index]);
      }
    }
    return res;
  }

  size_t getMaxFunctionArity() const
    {return actionSubsets.size() - 2;}

protected:
  std::vector<DiscreteDomainPtr> actionSubsets;
  // actionSubsets[i] with i <= maxFunctionArity ==> the actions with corresponding arity
  // actionSubsets[maxFunctionArity+1] ==> a singleton containing the yield action

  std::map<std::vector<size_t>, DiscreteDomainPtr> actionSets; // vector of action subset indices => union of the corresponding action subsets 
};

typedef ReferenceCountedObjectPtr<ExpressionActionDomainsCache> ExpressionActionDomainsCachePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_ACTION_DOMAINS_CACHE_H_
