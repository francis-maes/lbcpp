/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInferenceLearner.h    | A batch learner to learn a      |
| Author  : Francis Maes                   | DecoratorInference              |
| Started : 14/07/2010 12:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_META_DECORATOR_LEARNER_H_
# define LBCPP_INFERENCE_META_DECORATOR_LEARNER_H_

# include <lbcpp/Inference/Inference.h>
# include "../InferenceCallback/OnlineLearningInferenceCallback.h"

namespace lbcpp
{

class DecoratorInferenceLearner : public DecoratorInference
{
public:
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
  class CommonTypeFinder
  {
  public:
    void update(TypePtr newType)
    {
      if (!type)
        type = newType;
      else
      {
        if (newType->inheritsFrom(type))
          return;
        if (type->inheritsFrom(newType))
          type = newType;
        else
        {
          ErrorHandler::error(T("CommonTypeFinder"), T("No common type"));
        }
      }
    }

    void update(const Variable& variable)
      {if (!variable.isNil()) update(variable.getType());}

    TypePtr get() const
      {return type;}

  private:
    TypePtr type;
  };

  virtual ContainerPtr createSubTrainingData(InferenceContextPtr context, StaticDecoratorInferencePtr targetInference, ContainerPtr trainingData, ReturnCode& returnCode)
  {
    CommonTypeFinder subInputType;
    CommonTypeFinder subSupervisionType;

    size_t n = trainingData->size();
    std::vector<Variable> subTrainingData;
    for (size_t i = 0; i < n; ++i)
    {
      Variable inputAndSupervision = trainingData->getVariable(i);
      Inference::ReturnCode returnCode = Inference::finishedReturnCode;
      DecoratorInferenceStatePtr state = targetInference->prepareInference(context, inputAndSupervision[0], inputAndSupervision[1], returnCode);
      if (returnCode != Inference::finishedReturnCode)
        return ContainerPtr();
      
      Variable subInput = state->getSubInput();
      subInputType.update(subInput);
      Variable subSupervision = state->getSubSupervision();
      subSupervisionType.update(subSupervision);
      subTrainingData.push_back(Variable::pair(subInput, subSupervision));
    }

    VectorPtr res = new Vector(pairType(subInputType.get(), subSupervisionType.get()), n);
    for (size_t i = 0; i < n; ++i)
      res->setVariable(i, subTrainingData[i]);
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
