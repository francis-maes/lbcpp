/*-----------------------------------------.---------------------------------.
| Filename: WeightBoostingLearner.h        | Base class AdaBoost style       |
| Author  : Francis Maes                   |  algorithms                     |
| Started : 21/11/2011 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_
# define LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_

# include <lbcpp/Luape/LuapeLearner.h>

namespace lbcpp
{

class WeightBoostingLearner : public BoostingLearner
{
public:
  WeightBoostingLearner(BoostingWeakLearnerPtr weakLearner)
   : BoostingLearner(weakLearner) {}
  WeightBoostingLearner() {}

  virtual DenseDoubleVectorPtr computeSampleWeights(ExecutionContext& context, const VectorPtr& predictions, double& loss) const = 0;
  virtual VectorPtr makeSupervisions(const std::vector<ObjectPtr>& examples) const = 0;
  
  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
  {
    if (!BoostingLearner::setExamples(context, isTrainingData, data))
      return false;
     
    if (isTrainingData)
      supervisions = makeSupervisions(data);
    return true;
  }

  virtual bool doLearningIteration(ExecutionContext& context)
  {
    VectorPtr predictions = getTrainingPredictions();
    double loss;
    weights = computeSampleWeights(context, predictions, loss);
    context.resultCallback(T("loss"), loss);
    return BoostingLearner::doLearningIteration(context);
  }

protected:
  DenseDoubleVectorPtr weights;
  VectorPtr supervisions;
};

typedef ReferenceCountedObjectPtr<WeightBoostingLearner> WeightBoostingLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_WEIGHT_BOOSTING_H_
