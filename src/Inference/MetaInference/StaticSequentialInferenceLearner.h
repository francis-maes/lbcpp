/*-----------------------------------------.---------------------------------.
| Filename: StaticSequentialInferenceLearner.h| A batch learner that         |
| Author  : Francis Maes                   |  sequentially learns            |
| Started : 26/05/2010 19:08               |  it sub-inferences              |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_META_STATIC_SEQUENTIAL_LEARNER_H_
# define LBCPP_INFERENCE_META_STATIC_SEQUENTIAL_LEARNER_H_

# include <lbcpp/Inference/SequentialInference.h>
# include "RunOnSupervisedExamplesInference.h"

namespace lbcpp
{

class StaticSequentialInferenceLearner : public SequentialInference
{
public:
  virtual TypePtr getInputType() const
    {return pairType(staticSequentialInferenceClass(), containerClass(pairType(anyType(), anyType())));}

  virtual TypePtr getSupervisionType() const
    {return nilType();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return nilType();}

  virtual String getDescription(const Variable& input, const Variable& supervision) const
  {
    InferencePtr targetInference = input[0].getObjectAndCast<Inference>();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();
    return T("Sequential Learning of ") + targetInference->getName() + T(" with ") + 
      String((int)trainingData->getNumElements()) + T(" ") + trainingData->getElementsType()->getTemplateArgument(0)->getName() + T("(s)");
  }

  virtual SequentialInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    StaticSequentialInferencePtr targetInference = input[0].getObjectAndCast<StaticSequentialInference>();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();
    jassert(targetInference && trainingData);

    size_t n = trainingData->getNumElements();

    StatePtr res = new State(input, supervision);
    if (!targetInference->getNumSubInferences())
      return res;

    // for each training example: make initial target state
    res->targetStates.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable inputAndSupervision = trainingData->getElement(i);
      SequentialInferenceStatePtr targetState = targetInference->prepareInference(context, inputAndSupervision[0], inputAndSupervision[1], returnCode);
      if (!targetState)
        return SequentialInferenceStatePtr();
      res->targetStates[i] = targetState;
    }
    
    // set sub-learning inference
    setSubLearningInference(targetInference, res, 0);
    return res;
  }

  // returns false if the final state is reached
  virtual bool updateInference(InferenceContextPtr context, SequentialInferenceStatePtr s, ReturnCode& returnCode)
  {
    StatePtr state = s.staticCast<State>();
    Variable input = state->getInput();
    StaticSequentialInferencePtr targetInference = input[0].getObjectAndCast<StaticSequentialInference>();
    jassert(targetInference);

    ContainerPtr subTrainingData = state->getSubInput()[1].getObjectAndCast<Container>();
    
    int index = state->getStepNumber(); 
    jassert(index >= 0);
    ++index;
    if (index < (int)targetInference->getNumSubInferences())
    {
       // evaluate sub-inference and update currentObjects
      InferencePtr evaluateStepOnSubTrainingData = new RunSequentialInferenceStepOnExamples(targetInference, state->targetStates);
      context->run(evaluateStepOnSubTrainingData, subTrainingData, ObjectPtr(), returnCode);

      setSubLearningInference(targetInference, state, index);
      return true;
    }
    else
      return false; // final state
  }

private:
  struct State : public SequentialInferenceState
  {
    State(const Variable& input, const Variable& supervision)
      : SequentialInferenceState(input, supervision) {}

    std::vector<SequentialInferenceStatePtr> targetStates;
  };
  typedef ReferenceCountedObjectPtr<State> StatePtr;

  struct SubTrainingData : public Container
  {
    SubTrainingData(InferencePtr subInference, std::vector<SequentialInferenceStatePtr>& targetStates)
      : subInference(subInference), targetStates(targetStates) {}

    virtual ClassPtr getClass() const
      {return containerClass(pairType(anyType(), anyType()));}

    virtual TypePtr getElementsType() const
      {return pairType(subInference->getInputType(), subInference->getSupervisionType());}

    virtual size_t getNumElements() const
      {return targetStates.size();}

    virtual Variable getElement(size_t index) const
    {
      jassert(index < targetStates.size());
      SequentialInferenceStatePtr targetState = targetStates[index];
      return Variable::pair(targetState->getSubInput(), targetState->getSubSupervision());
    }

    virtual void setElement(size_t index, const Variable& value)
      {jassert(false);}

  private:
    InferencePtr subInference;
    std::vector<SequentialInferenceStatePtr>& targetStates;
  };

  void setSubLearningInference(StaticSequentialInferencePtr targetInference, StatePtr state, size_t index)
  {
    // get sub-learner
    InferencePtr targetSubInference = targetInference->getSubInference(index);
    InferencePtr subLearner = targetSubInference->getBatchLearner();
    if (!subLearner)
      subLearner = dummyInferenceLearner();

    // create sub-learning inference
    ContainerPtr subTrainingData = new SubTrainingData(targetSubInference, state->targetStates);
    state->setSubInference(subLearner, Variable::pair(targetSubInference, subTrainingData), Variable());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_STATIC_SEQUENTIAL_LEARNER_H_
