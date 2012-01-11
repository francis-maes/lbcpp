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

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_RANDOM_SEQUENTIAL_H_
