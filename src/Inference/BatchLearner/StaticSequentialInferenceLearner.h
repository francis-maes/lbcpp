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
# include <lbcpp/Inference/InferenceBatchLearner.h>

namespace lbcpp
{

class RunSequentialInferenceStepOnExamples : public ParallelInference
{
public:
  RunSequentialInferenceStepOnExamples(SequentialInferencePtr inference, std::vector<SequentialInferenceStatePtr>& currentStates)
    : ParallelInference(T("RunSequentialInferenceStepOnExamples")), inference(inference), currentStates(currentStates) {}

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    size_t n = learnerInput->getNumExamples();
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      const std::pair<Variable, Variable>& example = learnerInput->getExample(i);
      res->addSubInference(currentStates[i]->getSubInference(), example.first, example.second);
    }
    return res;
  }

  virtual Variable finalizeInference(ExecutionContext& context, ParallelInferenceStatePtr state) const
  {
    for (size_t i = 0; i < state->getNumSubInferences(); ++i)
    {
      currentStates[i]->setSubOutput(state->getSubOutput(i));
      if (!inference->updateInference(context, currentStates[i]))
        currentStates[i]->setFinalState();
    }
    return Variable();
  }

  virtual String getDescription(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    const InferencePtr& targetInference = learnerInput->getTargetInference();
    return T("Run ") + targetInference->getName() + T(" step with ") + 
      String((int)learnerInput->getNumExamples()) + T(" ") + targetInference->getInputType()->getName() + T("(s)");
  }

private:
  SequentialInferencePtr inference;
  std::vector<SequentialInferenceStatePtr>& currentStates;
};

class StaticSequentialInferenceLearner : public InferenceBatchLearner<SequentialInference>
{
public:
  virtual ClassPtr getTargetInferenceClass() const
    {return staticSequentialInferenceClass;}

  virtual SequentialInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    const StaticSequentialInferencePtr& targetInference = learnerInput->getTargetInference().staticCast<StaticSequentialInference>();
    jassert(targetInference);

    size_t n = learnerInput->getNumExamples();

    StatePtr res = new State(input, supervision);
    if (!targetInference->getNumSubInferences())
      return res;

    // for each training example: make initial target state
    res->numTrainingExamples = learnerInput->getNumTrainingExamples();
    res->numValidationExamples = learnerInput->getNumValidationExamples();
    res->targetStates.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
      const std::pair<Variable, Variable>& example = learnerInput->getExample(i);
      SequentialInferenceStatePtr targetState = targetInference->prepareInference(context, example.first, example.second);
      if (!targetState)
        return SequentialInferenceStatePtr();
      res->targetStates[i] = targetState;
    }
    
    // set sub-learning inference
    setSubLearningInference(targetInference, res, 0);
    return res;
  }

  // returns false if the final state is reached
  virtual bool updateInference(ExecutionContext& context, SequentialInferenceStatePtr s) const
  {
    StatePtr state = s.staticCast<State>();
    const InferenceBatchLearnerInputPtr& input = state->getInput().getObjectAndCast<InferenceBatchLearnerInput>(context);
    const StaticSequentialInferencePtr& targetInference = input->getTargetInference().staticCast<StaticSequentialInference>();
    jassert(targetInference);

    const InferenceBatchLearnerInputPtr& subInput = state->getSubInput().getObjectAndCast<InferenceBatchLearnerInput>(context);
    
    int index = state->getStepNumber(); 
    jassert(index >= 0);
    ++index;
    if (index < (int)targetInference->getNumSubInferences())
    {
       // evaluate sub-inference and update currentObjects
      InferencePtr evaluateStepOnSubTrainingData = new RunSequentialInferenceStepOnExamples(targetInference, state->targetStates);
      if (!runInference(context, evaluateStepOnSubTrainingData, subInput, ObjectPtr()))
        return false;

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
      : SequentialInferenceState(input, supervision), numTrainingExamples(0), numValidationExamples(0) {}

    std::vector<SequentialInferenceStatePtr> targetStates;
    size_t numTrainingExamples;
    size_t numValidationExamples;
  };
  typedef ReferenceCountedObjectPtr<State> StatePtr;

  void setSubLearningInference(const StaticSequentialInferencePtr& targetInference, const StatePtr& state, size_t index) const
  {
    // get sub-learner
    InferencePtr targetSubInference = targetInference->getSubInference(index);
    InferencePtr subLearner = targetSubInference->getBatchLearner();
    if (!subLearner)
      subLearner = dummyInferenceLearner();

    // create sub-learning inference
    InferenceBatchLearnerInputPtr subLearnerInput = new InferenceBatchLearnerInput(targetSubInference, state->numTrainingExamples, state->numValidationExamples);
    size_t n = state->targetStates.size();
    jassert(n == state->numTrainingExamples + state->numValidationExamples);
    for (size_t i = 0; i < n; ++i)
    {
      const SequentialInferenceStatePtr& targetState = state->targetStates[i];
      subLearnerInput->setExample(i, targetState->getSubInput(), targetState->getSubSupervision());
    }

    state->setSubInference(subLearner, subLearnerInput, Variable());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_STATIC_SEQUENTIAL_H_
