/*-----------------------------------------.---------------------------------.
| Filename: PrecomputePerceptionsInfere...h| A decorator learner that        |
| Author  : Francis Maes                   |  precomputes Perceptions        |
| Started : 01/11/2010 15:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_PRECOMPUTE_PERCEPTIONS_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_PRECOMPUTE_PERCEPTIONS_H_

# include <lbcpp/Data/Pair.h>
# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>

namespace lbcpp
{

class PrecomputePerceptionsInferenceLearner : public InferenceBatchLearner<StaticDecoratorInference>
{
public:
  PrecomputePerceptionsInferenceLearner(InferencePtr baseLearner)
    {setSubInference(baseLearner);}
  PrecomputePerceptionsInferenceLearner() {}

  virtual const PerceptionPtr& getPerception(const InferencePtr& targetInference) const = 0;

  virtual DecoratorInferenceStatePtr prepareInference(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    InferencePtr targetInference = getInference(input);
    ContainerPtr trainingData = getTrainingData(input);
    const PerceptionPtr& perception = getPerception(targetInference);

    TypePtr elementsType = pairClass(perception->getOutputType(), targetInference->getSupervisionType());
    size_t n = trainingData->getNumElements();
    ContainerPtr precomputedTrainingData = vector(elementsType, n);
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr example = trainingData->getElement(i).getObjectAndCast<Pair>();
      precomputedTrainingData->setElement(i, new Pair(elementsType, perception->compute(example->getFirst()), example->getSecond()));
    }

    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    res->setSubInference(decorated, Variable::pair(targetInference, precomputedTrainingData), supervision);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_PRECOMPUTE_PERCEPTIONS_H_
