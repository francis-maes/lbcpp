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
  WeightBoostingLearner(WeakLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
   : BoostingLearner(weakLearner, maxIterations, treeDepth) {}
  WeightBoostingLearner() {}

  virtual DenseDoubleVectorPtr computeSampleWeights(ExecutionContext& context, const LuapeInferencePtr& problem, double& loss) const = 0;
  
  virtual bool doLearningIteration(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double& trainingScore, double& validationScore)
  {
    double loss;
    weights = computeSampleWeights(context, problem, loss);
    context.resultCallback(T("loss"), loss);
    return BoostingLearner::doLearningIteration(context, node, problem, examples, trainingScore, validationScore);
  }

protected:
  DenseDoubleVectorPtr weights;
};

typedef ReferenceCountedObjectPtr<WeightBoostingLearner> WeightBoostingLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_
