/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInferenceLearner.h    | A batch learner to learn a      |
| Author  : Francis Maes                   | DecoratorInference              |
| Started : 14/07/2010 12:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_DECORATOR_LEARNER_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_DECORATOR_LEARNER_H_

# include <lbcpp/Data/Vector.h>
# include <lbcpp/Inference/DecoratorInference.h>

namespace lbcpp
{

class DecoratorInferenceLearner : public InferenceLearner<DecoratorInference>
{
public:
  virtual ClassPtr getTargetInferenceClass() const
    {return staticDecoratorInferenceClass;}

  virtual DecoratorInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    StaticDecoratorInferencePtr targetInference = getInferenceAndCast<StaticDecoratorInference>(input);
    ContainerPtr trainingData = getTrainingData(input);
    jassert(targetInference && trainingData);
    
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);

    InferencePtr targetSubInference = targetInference->getSubInference();
    if (targetSubInference && targetSubInference->getBatchLearner())
    {
      ContainerPtr subTrainingData = createSubTrainingData(context, targetInference, trainingData, returnCode);
      if (returnCode != finishedReturnCode)
        return DecoratorInferenceStatePtr();

      res->setSubInference(targetSubInference->getBatchLearner(), Variable::pair(targetSubInference, subTrainingData), Variable());
    }
    return res;
  }

protected:
  virtual ContainerPtr createSubTrainingData(InferenceContextPtr context, StaticDecoratorInferencePtr targetInference, ContainerPtr trainingData, ReturnCode& returnCode)
  {
    size_t n = trainingData->getNumElements();
    InferencePtr targetSubInference = targetInference->getSubInference();
    TypePtr pairType = pairClass(targetSubInference->getInputType(), targetSubInference->getSupervisionType());
    VectorPtr res = vector(pairType, n);

    for (size_t i = 0; i < n; ++i)
    {
      Variable inputAndSupervision = trainingData->getElement(i);
      Inference::ReturnCode returnCode = Inference::finishedReturnCode;
      DecoratorInferenceStatePtr state = targetInference->prepareInference(context, inputAndSupervision[0], inputAndSupervision[1], returnCode);
      if (returnCode != Inference::finishedReturnCode)
        return ContainerPtr();
      res->setElement(i, Variable::pair(state->getSubInput(), state->getSubSupervision(), pairType));
    }
    return res;
  }
};

class PostProcessInferenceLearner : public DecoratorInferenceLearner
{
protected:
  virtual ContainerPtr createSubTrainingData(InferenceContextPtr context, StaticDecoratorInferencePtr targetInference, ContainerPtr trainingData, ReturnCode& returnCode)
    {return trainingData;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_DECORATOR_LEARNER_H_
