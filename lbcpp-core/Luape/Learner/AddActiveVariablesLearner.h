/*-----------------------------------------.---------------------------------.
| Filename: AddActiveVariablesLearner.h    | A decorator that dynamically    |
| Author  : Francis Maes                   | add active variables            |
| Started : 11/01/2012 12:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_ADD_ACTIVE_VARIABLES_H_
# define LBCPP_LUAPE_LEARNER_ADD_ACTIVE_VARIABLES_H_

# include <lbcpp/Luape/ExpressionBuilder.h>

namespace lbcpp
{

class AddActiveVariablesLearner : public DecoratorLearner
{
public:
  AddActiveVariablesLearner(LuapeLearnerPtr decorated, size_t numActiveVariables, bool deterministic)
    : DecoratorLearner(decorated), numActiveVariables(numActiveVariables), deterministic(deterministic) {}
  AddActiveVariablesLearner() : numActiveVariables(0), deterministic(false) {}

  virtual ExpressionPtr learn(ExecutionContext& context, const ExpressionPtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    std::map<ExpressionPtr, double> importances;
    getImportances(problem->getRootNode(), importances);
    
    problem->clearActiveVariables();

    if (deterministic)
    {
      std::multimap<double, ExpressionPtr> nodeImportanceMap;
      for (std::map<ExpressionPtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it)
        nodeImportanceMap.insert(std::make_pair(it->second, it->first));
      std::multimap<double, ExpressionPtr>::reverse_iterator it = nodeImportanceMap.rbegin();

      while (problem->getNumActiveVariables() < numActiveVariables && it != nodeImportanceMap.rend())
      {
        ExpressionPtr node = it->second;
        if (!node.isInstanceOf<VariableExpression>())
          addActiveVariable(context, problem, node, it->first);
        ++it;
      }
    }
    else
    {
      double Z = 0.0;
      std::vector<double> probabilities(importances.size());
      std::vector<ExpressionPtr> nodes(importances.size());
      size_t index = 0;
      for (std::map<ExpressionPtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it, ++index)
      {
        Z += it->second;
        probabilities[index] = it->second;
        nodes[index] = it->first;
      }

      while (problem->getNumActiveVariables() < numActiveVariables && Z > 1e-12)
      {
        jassert(isNumberValid(Z));
        size_t index = context.getRandomGenerator()->sampleWithProbabilities(probabilities, Z);
        ExpressionPtr node = nodes[index];
        if (!node.isInstanceOf<VariableExpression>())
          addActiveVariable(context, problem, node, probabilities[index]);
        Z -= probabilities[index];
        probabilities[index] = 0.0;
      }
    }
    return DecoratorLearner::learn(context, node, problem, examples);
  }

protected:
  friend class AddActiveVariablesLearnerClass;

  size_t numActiveVariables;
  bool deterministic;
  
  void getImportances(const ExpressionPtr& node, std::map<ExpressionPtr, double>& res) const
  {
    if (node && res.find(node) == res.end())
    {
      double importance = node->getImportance();
      jassert(isNumberValid(importance));
      if (importance > 0)
        if (!node.isInstanceOf<FunctionExpression>() || node.staticCast<FunctionExpression>()->getFunction()->getClassName() != T("StumpFunction"))
          res[node] = importance;
      size_t n = node->getNumSubNodes();
      for (size_t i = 0; i < n; ++i)
        getImportances(node->getSubNode(i), res);
    }
  }

  void addActiveVariable(ExecutionContext& context, const LuapeInferencePtr& function, const ExpressionPtr& variable, double importance)
  {
    function->addActiveVariable(variable);
    if (verbose)
      context.informationCallback(T("Active variable: ") + variable->toShortString() + T(" [") + String(importance, 2) + T("]"));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ADD_ACTIVE_VARIABLES_H_

