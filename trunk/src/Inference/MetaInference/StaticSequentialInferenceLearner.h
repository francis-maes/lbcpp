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

class StaticSequentialInferenceLearner : public InferenceLearner<SequentialInference>
{
public:
  virtual ClassPtr getTargetInferenceClass() const
    {return staticSequentialInferenceClass;}

  virtual SequentialInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    StaticSequentialInferencePtr targetInference = getInferenceAndCast<StaticSequentialInference>(input);
    ContainerPtr trainingData = getTrainingData(input);
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
    StaticSequentialInferencePtr targetInference = getInferenceAndCast<StaticSequentialInference>(input);
    jassert(targetInference);

    ContainerPtr subTrainingData = getTrainingData(state->getSubInput());
    
    int index = state->getStepNumber(); 
    jassert(index >= 0);
    ++index;
    if (index < (int)targetInference->getNumSubInferences())
    {
       // evaluate sub-inference and update currentObjects
      InferencePtr evaluateStepOnSubTrainingData = new RunSequentialInferenceStepOnExamples(targetInference, state->targetStates);
      context->run(evaluateStepOnSubTrainingData.get(), subTrainingData, ObjectPtr(), returnCode);

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
      : subInference(subInference), targetStates(targetStates), pairType(pairClass(subInference->getInputType(), subInference->getSupervisionType())) {}

    virtual ClassPtr getClass() const
      {return containerClass(pairClass(anyType, anyType));}

    virtual TypePtr getElementsType() const
      {return pairType;}

    virtual size_t getNumElements() const
      {return targetStates.size();}

    virtual Variable getElement(size_t index) const
    {
      jassert(index < targetStates.size());
      SequentialInferenceStatePtr targetState = targetStates[index];
      return Variable::pair(targetState->getSubInput(), targetState->getSubSupervision(), pairType);
    }

    virtual void setElement(size_t index, const Variable& value)
      {jassert(false);}

  private:
    InferencePtr subInference;
    std::vector<SequentialInferenceStatePtr>& targetStates;
    TypePtr pairType;
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
