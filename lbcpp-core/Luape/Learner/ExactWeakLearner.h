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
  ExactWeakLearner(ExpressionBuilderPtr nodeBuilder)
    : NodeBuilderBasedLearner(nodeBuilder) {}
  ExactWeakLearner() {}

  virtual ExpressionPtr learn(ExecutionContext& context, const ExpressionPtr& node, const ExpressionDomainPtr& problem, const IndexSetPtr& examples)
  {
    // make weak nodes
    std::vector<ExpressionPtr> weakNodes;
    nodeBuilder->buildNodes(context, problem, 0, weakNodes);
    if (!weakNodes.size())
      return ExpressionPtr();

    // evaluate each weak node
    std::vector<ExpressionPtr> bestWeakNodes;
    bestObjectiveValue = -DBL_MAX;
    for (size_t i = 0; i < weakNodes.size(); ++i)
    {
      ExpressionPtr weakNode = weakNodes[i];
      double objectiveValue = computeObjective(context, problem, examples, weakNode); // side effect on weakNode
      if (objectiveValue >= bestObjectiveValue)
      {
        if (objectiveValue > bestObjectiveValue)
        {
          bestObjectiveValue = objectiveValue;
          bestWeakNodes.clear();
        }
        bestWeakNodes.push_back(weakNode);
      }
    }
    if (bestWeakNodes.empty())
      return ExpressionPtr();
    if (bestWeakNodes.size() == 1)
      return bestWeakNodes[0];
    else
      return bestWeakNodes[context.getRandomGenerator()->sampleSize(bestWeakNodes.size())];
  }

  virtual double computeObjective(ExecutionContext& context, const ExpressionDomainPtr& problem, const IndexSetPtr& examples, ExpressionPtr& weakNode)
    {return objective->computeObjectiveWithEventualStump(context, problem, weakNode, examples);}
};


class RandomSplitWeakLearner : public ExactWeakLearner
{
public:
  RandomSplitWeakLearner(ExpressionBuilderPtr nodeBuilder)
    : ExactWeakLearner(nodeBuilder) {}
  RandomSplitWeakLearner() {}

  virtual double computeObjective(ExecutionContext& context, const ExpressionDomainPtr& problem, const IndexSetPtr& examples, ExpressionPtr& weakNode)
  {
    LuapeSampleVectorPtr samples = problem->getTrainingCache()->getSamples(context, weakNode, examples);
    if (samples->getElementsType() == booleanType)
      return objective->compute(samples);
     
    double minimumValue = DBL_MAX;
    double maximumValue = -DBL_MAX;
    bool isInteger = samples->getElementsType()->inheritsFrom(integerType);
    for (LuapeSampleVector::const_iterator it = samples->begin(); it != samples->end(); ++it)
    {
      double value = isInteger ? (double)it.getRawInteger() : it.getRawDouble();
      if (value < minimumValue)
        minimumValue = value;
      if (value > maximumValue)
        maximumValue = value;
    }
    
    if (maximumValue <= minimumValue)
      return -DBL_MAX;

    double threshold = context.getRandomGenerator()->sampleDouble(minimumValue, maximumValue);
    //context.informationCallback(T("min = ") + String(minimumValue) + T(" max = ") + String(maximumValue) + T(" threshold = ") + String(threshold));
    
    weakNode = new FunctionExpression(stumpFunction(threshold), weakNode);
    return objective->compute(problem->getTrainingCache()->getSamples(context, weakNode, examples));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_WEAK_LEARNER_EXACT_H_
