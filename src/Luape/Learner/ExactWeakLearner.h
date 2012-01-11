/*-----------------------------------------.---------------------------------.
| Filename: ExactWeakLearner.h             | Exact Weak Learner              |
| Author  : Francis Maes                   |                                 |
| Started : 03/01/2012 18:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_WEAK_LEARNER_EXACT_H_
# define LBCPP_LUAPE_WEAK_LEARNER_EXACT_H_

# include <lbcpp/Luape/LuapeLearner.h>

namespace lbcpp
{

class ExactWeakLearner : public NodeBuilderBasedLearner
{
public:
  ExactWeakLearner(LuapeNodeBuilderPtr nodeBuilder)
    : NodeBuilderBasedLearner(nodeBuilder) {}
  ExactWeakLearner() {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    // make weak nodes
    std::vector<LuapeNodePtr> weakNodes;
    nodeBuilder->buildNodes(context, problem, 0, weakNodes);
    if (!weakNodes.size())
      return LuapeNodePtr();

    // evaluate each weak node
    LuapeNodePtr bestWeakNode;
    bestObjectiveValue = -DBL_MAX;
    for (size_t i = 0; i < weakNodes.size(); ++i)
    {
      LuapeNodePtr weakNode = weakNodes[i];
      double objectiveValue = computeObjective(context, problem, examples, weakNode); // side effect on weakNode
      if (objectiveValue > bestObjectiveValue)
      {
        bestObjectiveValue = objectiveValue;
        bestWeakNode = weakNode;
      }
    }
    return bestWeakNode;
  }

  virtual double computeObjective(ExecutionContext& context, const LuapeInferencePtr& problem, const IndexSetPtr& examples, LuapeNodePtr& weakNode)
    {return objective->computeObjectiveWithEventualStump(context, problem, weakNode, examples);}
};


class RandomSplitWeakLearner : public ExactWeakLearner
{
public:
  RandomSplitWeakLearner(LuapeNodeBuilderPtr nodeBuilder)
    : ExactWeakLearner(nodeBuilder) {}
  RandomSplitWeakLearner() {}

  virtual double computeObjective(ExecutionContext& context, const LuapeInferencePtr& problem, const IndexSetPtr& examples, LuapeNodePtr& weakNode)
  {
    LuapeSampleVectorPtr samples = problem->getTrainingCache()->getSamples(context, weakNode, examples);
    double minimumValue = DBL_MAX;
    double maximumValue = -DBL_MAX;
    for (LuapeSampleVector::const_iterator it = samples->begin(); it != samples->end(); ++it)
    {
      double value = it.getRawDouble();
      if (value < minimumValue)
        minimumValue = value;
      if (value > maximumValue)
        maximumValue = value;
    }
    if (maximumValue < minimumValue)
      return -DBL_MAX;
    double threshold = context.getRandomGenerator()->sampleDouble(minimumValue, maximumValue);
    weakNode = new LuapeFunctionNode(stumpLuapeFunction(threshold), weakNode);
    return objective->compute(problem->getTrainingCache()->getSamples(context, weakNode, examples));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_EXACT_H_
