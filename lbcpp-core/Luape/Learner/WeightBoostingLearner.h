/*-----------------------------------------.---------------------------------.
| Filename: WeightBoostingLearner.h        | Base class AdaBoost style       |
| Author  : Francis Maes                   |  algorithms                     |
| Started : 21/11/2011 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_
# define LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_

# include "BoostingLearner.h"

namespace lbcpp
{

class WeightBoostingLearner : public BoostingLearner
{
public:
  WeightBoostingLearner(LearningObjectivePtr objective, LuapeLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
   : BoostingLearner(objective, weakLearner, maxIterations, treeDepth) {}
  WeightBoostingLearner() {}

  virtual DenseDoubleVectorPtr computeSampleWeights(ExecutionContext& context, const LuapeInferencePtr& problem, double& logLoss) const = 0;
  virtual void updateSampleWeights(ExecutionContext& context, const LuapeInferencePtr& problem, const ExpressionPtr& contribution, const DenseDoubleVectorPtr& weights, double& logLoss) const = 0;
  virtual Variable computeVote(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions) const = 0;
  virtual Variable negateVote(const Variable& vote) const = 0;
  virtual FunctionPtr makeVoteFunction(ExecutionContext& context, const LuapeInferencePtr& problem, const Variable& vote) const = 0;
  
  virtual bool initialize(ExecutionContext& context, const ExpressionPtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    if (!BoostingLearner::initialize(context, node, problem, examples))
      return false;
    objective->setWeights(computeSampleWeights(context, problem, logLoss));
    return true;
  }

  virtual void contributionAdded(ExecutionContext& context, const LuapeInferencePtr& problem, const ExpressionPtr& contribution)
    {updateSampleWeights(context, problem, contribution, objective.staticCast<SupervisedLearningObjective>()->getWeights(), logLoss);}

  virtual bool doLearningIteration(ExecutionContext& context, ExpressionPtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double& trainingScore, double& validationScore)
  {
    context.resultCallback(T("logLoss"), logLoss);
    context.resultCallback(T("loss"), pow(10.0, logLoss));
    return BoostingLearner::doLearningIteration(context, node, problem, examples, trainingScore, validationScore);
  }

  virtual ExpressionPtr turnWeakNodeIntoContribution(ExecutionContext& context, const ExpressionPtr& weakNode, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double weakObjective) const
  {
    const ExpressionUniversePtr& universe = problem->getUniverse();

    jassert(weakNode);
    LuapeSampleVectorPtr weakPredictions = problem->getTrainingCache()->getSamples(context, weakNode, examples);
    Variable vote = computeVote(context, problem, weakPredictions);
    if (!vote.exists())
      return ExpressionPtr();

    ExpressionPtr res;
    jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
    if (weakNode.isInstanceOf<ConstantExpression>())
    {
      ConstantExpressionPtr constantNode = weakNode.staticCast<ConstantExpression>();
      Variable constantValue = constantNode->getValue();
      jassert(constantValue.isBoolean() && constantValue.exists());
      if (constantValue.getBoolean())
        res = universe->makeConstantNode(vote);
      else
        res = universe->makeConstantNode(negateVote(vote));
    }
    else
    {
      FunctionPtr voteFunction = makeVoteFunction(context, problem, vote);
      res = universe->makeFunctionExpression(voteFunction, weakNode);
    }
    if (verbose)
      context.informationCallback(res->toShortString());
    return res;
  }

protected:
  double logLoss;
};

typedef ReferenceCountedObjectPtr<WeightBoostingLearner> WeightBoostingLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_
