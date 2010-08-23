/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInferenceLearner.h    | A batch learner to learn a      |
| Author  : Francis Maes                   | DecoratorInference              |
| Started : 14/07/2010 12:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_META_DECORATOR_LEARNER_H_
# define LBCPP_INFERENCE_META_DECORATOR_LEARNER_H_

# include <lbcpp/Data/Vector.h>
# include <lbcpp/Inference/Inference.h>
# include "../InferenceCallback/OnlineLearningInferenceCallback.h"

namespace lbcpp
{

class DecoratorInferenceLearner : public DecoratorInference
{
public:
  virtual TypePtr getInputType() const
    {return pairType(staticDecoratorInferenceClass(), containerClass());}

  virtual TypePtr getSupervisionType() const
    {return nilType();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return nilType();}

  virtual DecoratorInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    StaticDecoratorInferencePtr targetInference = input[0].getObjectAndCast<StaticDecoratorInference>();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();
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
    std::vector<Variable> subTrainingData;
    for (size_t i = 0; i < n; ++i)
    {
      Variable inputAndSupervision = trainingData->getElement(i);
      Inference::ReturnCode returnCode = Inference::finishedReturnCode;
      DecoratorInferenceStatePtr state = targetInference->prepareInference(context, inputAndSupervision[0], inputAndSupervision[1], returnCode);
      if (returnCode != Inference::finishedReturnCode)
        return ContainerPtr();
      
      subTrainingData.push_back(Variable::pair(state->getSubInput(), state->getSubSupervision()));
    }

    InferencePtr targetSubInference = targetInference->getSubInference();

    VectorPtr res = new Vector(pairType(targetSubInference->getInputType(), targetSubInference->getSupervisionType()), n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, subTrainingData[i]);
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

#endif // !LBCPP_INFERENCE_META_DECORATOR_LEARNER_H_
