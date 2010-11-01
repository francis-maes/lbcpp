/*-----------------------------------------.---------------------------------.
| Filename: StaticSequentialInferenceLearner.h| A batch learner that         |
| Author  : Francis Maes                   |  sequentially learns            |
| Started : 26/05/2010 19:08               |  it sub-inferences              |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_STATIC_SEQUENTIAL_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_STATIC_SEQUENTIAL_H_

# include <lbcpp/Inference/SequentialInference.h>

namespace lbcpp
{

class RunSequentialInferenceStepOnExamples : public ParallelInference
{
public:
  RunSequentialInferenceStepOnExamples(SequentialInferencePtr inference, std::vector<SequentialInferenceStatePtr>& currentStates)
    : ParallelInference(T("RunSequentialInferenceStepOnExamples")), inference(inference), currentStates(currentStates) {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ContainerPtr examples = input.dynamicCast<Container>();
    jassert(examples);

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    size_t n = examples->getNumElements();
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable example = examples->getElement(i);
      res->addSubInference(currentStates[i]->getSubInference(), example[0], example[1]);
    }
    return res;
  }

  virtual Variable finalizeInference(InferenceContextWeakPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    for (size_t i = 0; i < state->getNumSubInferences(); ++i)
    {
      currentStates[i]->setSubOutput(state->getSubOutput(i));
      if (!inference->updateInference(context, currentStates[i], returnCode))
        currentStates[i]->setFinalState();
      if (returnCode != finishedReturnCode)
        break;
    }
    return Variable();
  }

  virtual String getDescription(const Variable& input, const Variable& supervision) const
  {
    const ContainerPtr& examples = input.getObjectAndCast<Container>();
    return T("Run ") + inference->getName() + T(" step with ") + 
      String((int)examples->getNumElements()) + T(" ") + examples->getElementsType()->getName() + T("(s)");
  }

private:
  SequentialInferencePtr inference;
  std::vector<SequentialInferenceStatePtr>& currentStates;
};

class StaticSequentialInferenceLearner : public InferenceLearner<SequentialInference>
{
public:
  virtual ClassPtr getTargetInferenceClass() const
    {return staticSequentialInferenceClass;}

  virtual SequentialInferenceStatePtr prepareInference(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
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
  virtual bool updateInference(InferenceContextWeakPtr context, SequentialInferenceStatePtr s, ReturnCode& returnCode)
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

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_STATIC_SEQUENTIAL_H_
