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
  virtual void updateSampleWeights(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& contribution, const DenseDoubleVectorPtr& weights, double& logLoss) const = 0;
  
  virtual bool initialize(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    if (!BoostingLearner::initialize(context, node, problem, examples))
      return false;
    objective->setWeights(computeSampleWeights(context, problem, logLoss));
    return true;
  }

  virtual void contributionAdded(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& contribution)
    {updateSampleWeights(context, problem, contribution, objective.staticCast<SupervisedLearningObjective>()->getWeights(), logLoss);}

  virtual bool doLearningIteration(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double& trainingScore, double& validationScore)
  {
    context.resultCallback(T("logLoss"), logLoss);
    context.resultCallback(T("loss"), pow(10.0, logLoss));
    return BoostingLearner::doLearningIteration(context, node, problem, examples, trainingScore, validationScore);
  }

protected:
  double logLoss;
};

typedef ReferenceCountedObjectPtr<WeightBoostingLearner> WeightBoostingLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_
