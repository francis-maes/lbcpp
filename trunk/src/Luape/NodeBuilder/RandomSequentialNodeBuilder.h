/*-----------------------------------------.---------------------------------.
| Filename: RandomSequentialNodeBuilder.h  | Random Sampler of Nodes         |
| Author  : Francis Maes                   |                                 |
| Started : 04/01/2012 17:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_RANDOM_SEQUENTIAL_H_
# define LBCPP_LUAPE_NODE_BUILDER_RANDOM_SEQUENTIAL_H_

# include "NodeBuilderTypeSearchSpace.h"
# include <lbcpp/Luape/LuapeNodeBuilder.h>

namespace lbcpp
{

class RandomSequentialNodeBuilder : public SequentialNodeBuilder
{
public:
  RandomSequentialNodeBuilder(size_t numNodes, size_t complexity)
    : SequentialNodeBuilder(numNodes, complexity) {}
  RandomSequentialNodeBuilder() {}

  virtual bool sampleAction(ExecutionContext& context, const LuapeInferencePtr& problem, LuapeGraphBuilderTypeStatePtr typeState, ObjectPtr& res) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    if (!typeState)
      return false;

    std::vector<double> probabilities(3, 0.0);
    double Z = 0.0;
    if (typeState->hasPushActions())
      probabilities[0] = 1.0, ++Z;
    if (typeState->hasApplyActions())
      probabilities[1] = 1.0, ++Z;
    if (typeState->hasYieldAction())
      probabilities[2] = 1.0, ++Z;
    jassert(Z > 0.0);
    size_t actionKind = random->sampleWithProbabilities(probabilities, Z);

    switch (actionKind)
    {
    case 0: // push
      {
        static const size_t numTrials = 10;
        size_t numVariables = problem->getNumInputs() + problem->getNumActiveVariables();
        for (size_t trial = 0; trial < numTrials; ++trial)
        {
          size_t variableIndex = random->sampleSize(numVariables);
          LuapeNodePtr variable = variableIndex < problem->getNumInputs()
            ? (LuapeNodePtr)problem->getInput(variableIndex)
            : problem->getActiveVariable(variableIndex - problem->getNumInputs());
          if (typeState->hasPushAction(variable->getType()))
          {
            res = variable;
            return true;
          }
        }
      }

    case 1: // apply
      {
        const std::vector<std::pair<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr> >& apply = typeState->getApplyActions();
        jassert(apply.size());
        if (apply.empty())
          return false;
        res = apply[random->sampleSize(apply.size())].first;
        return true;
      }

    case 2: // yield
      res = ObjectPtr();
      return true;
    };

    return false;
  }
};

class BiasedRandomSequentialNodeBuilder : public SequentialNodeBuilder
{
public:
  BiasedRandomSequentialNodeBuilder(size_t numNodes, size_t complexity, double initialImportance)
    : SequentialNodeBuilder(numNodes, complexity), initialImportance(initialImportance), counter((size_t)-1) {}
  BiasedRandomSequentialNodeBuilder() : counter((size_t)-1) {}

  virtual void buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<LuapeNodePtr>& res)
  {
    if (!function->getRootNode() || function->getRootNode()->getNumSubNodes() != counter)
    {
      if (function->getRootNode())
        counter = function->getRootNode()->getNumSubNodes();

      std::map<LuapeNodePtr, double> importances;
      for (size_t i = 0; i < function->getNumInputs(); ++i)
        importances[function->getInput(i)] = function->getInput(i)->getImportance();
      if (function->getRootNode())
        LuapeUniverse::getImportances(function->getRootNode(), importances);
      else
        function->getUniverse()->getImportances(importances);
   
      Z = 0.0;
      probabilities.resize(importances.size());
      nodes.resize(importances.size());
      size_t index = 0;
      for (std::map<LuapeNodePtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it, ++index)
      {
        double p = it->second;
        if (it->first.isInstanceOf<LuapeInputNode>())
          p += initialImportance;
        Z += p;
        probabilities[index] = p;
        nodes[index] = it->first;
      }

      //LuapeUniverse::displayMostImportantNodes(context, importances);
    }
    
    SequentialNodeBuilder::buildNodes(context, function, maxCount, res);
  }

  virtual bool sampleAction(ExecutionContext& context, const LuapeInferencePtr& problem, LuapeGraphBuilderTypeStatePtr typeState, ObjectPtr& res) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    if (!typeState)
      return false;

    std::vector<double> probabilities(3, 0.0);
    double Z = 0.0;
    if (typeState->hasPushActions())
      probabilities[0] = 1.0, ++Z;
    if (typeState->hasApplyActions())
      probabilities[1] = 1.0, ++Z;
    if (typeState->hasYieldAction())
      probabilities[2] = 1.0, ++Z;
    jassert(Z > 0.0);
    size_t actionKind = random->sampleWithProbabilities(probabilities, Z);

    switch (actionKind)
    {
    case 0: // push
      {
        static const size_t numTrials = 100;
        for (size_t trial = 0; trial < numTrials; ++trial)
        {
          size_t nodeIndex = random->sampleWithProbabilities(this->probabilities, this->Z);
          LuapeNodePtr node = this->nodes[nodeIndex];
          if (typeState->hasPushAction(node->getType()))
          {
            res = node;
            return true;
          }
        }
      }
      return false;

    case 1: // apply
      {
        const std::vector<std::pair<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr> >& apply = typeState->getApplyActions();
        jassert(apply.size());
        if (apply.empty())
          return false;
        res = apply[random->sampleSize(apply.size())].first;
        return true;
      }

    case 2: // yield
      res = ObjectPtr();
      return true;
    };

    return false;
  }

protected:
  friend class BiasedRandomSequentialNodeBuilderClass;

  double initialImportance;
  size_t counter;

  double Z;
  std::vector<double> probabilities;
  std::vector<LuapeNodePtr> nodes;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_RANDOM_SEQUENTIAL_H_
