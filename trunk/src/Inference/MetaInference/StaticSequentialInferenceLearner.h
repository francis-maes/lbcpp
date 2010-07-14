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
    {return pairType(staticSequentialInferenceClass(), containerClass());}

  virtual TypePtr getSupervisionType() const
    {return nilType();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return nilType();}

  virtual SequentialInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    StaticSequentialInferencePtr targetInference = input[0].getObjectAndCast<StaticSequentialInference>();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();
    jassert(targetInference && trainingData);

    size_t n = trainingData->size();

    StatePtr res = new State(input, supervision);
    if (!targetInference->getNumSubInferences())
      return res;

    // for each training example: make initial target state
    res->targetStates.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable inputAndSupervision = trainingData->getVariable(i);
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
     // evaluate sub-inference and update currentObjects
    InferencePtr evaluateStepOnSubTrainingData = new RunSequentialInferenceStepOnExamples(targetInference, state->targetStates);
    context->runInference(evaluateStepOnSubTrainingData, subTrainingData, ObjectPtr(), returnCode);
    
    int index = state->getStepNumber(); 
    jassert(index >= 0);
    ++index;
    if (index < (int)targetInference->getNumSubInferences())
    {
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
      {return containerClass();}

    virtual TypePtr getElementsType() const
      {return pairType(subInference->getInputType(), subInference->getSupervisionType());}

    virtual size_t getNumVariables() const
      {return targetStates.size();}

    virtual Variable getVariable(size_t index) const
    {
      jassert(index < targetStates.size());
      SequentialInferenceStatePtr targetState = targetStates[index];
      return Variable::pair(targetState->getSubInput(), targetState->getSubSupervision());
    }

    virtual void setVariable(size_t index, const Variable& value)
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
/*

class SequentialInferenceLearner : public Inference
{
public:
  virtual TypePtr getInputType() const
    {return pairType(staticSequentialInferenceClass(), containerClass());}

  virtual TypePtr getSupervisionType() const
    {return nilType();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return nilType();}

  ReturnCode train(InferenceContextPtr context, InferencePtr inf, ContainerPtr trainingData)
  {
    size_t numTrainingExamples = trainingData->size();

    SequentialInferencePtr inference = inf.dynamicCast<SequentialInference>();
    jassert(inference);

    // prepare inferences
    ReturnCode returnCode = finishedReturnCode;
    std::vector<SequentialInferenceStatePtr> currentStates(numTrainingExamples);
    for (size_t i = 0; i < numTrainingExamples; ++i)
    {
      Variable example = trainingData->getVariable(i);
      jassert(example && example.size() == 2 && example[0]);
      SequentialInferenceStatePtr state = inference->prepareInference(context, example[0], example[1], returnCode);
      if (returnCode != finishedReturnCode)
        return returnCode;
      jassert(state->getSubInput());
      currentStates[i] = state;
    }

    size_t stepNumber = 0;
    while (true)
    {
      // make sub-training data 
      ContainerPtr subTrainingData = new Vector(pairType(), numTrainingExamples);
      for (size_t i = 0; i < numTrainingExamples; ++i)
      {
        Variable subInput = currentStates[i]->getSubInput();
        Variable subSupervision = currentStates[i]->getSubSupervision();
        jassert(subInput);
        subTrainingData->setVariable(i, Variable::pair(subInput, subSupervision));
      }

      // get sub-inference and sub-learner
      InferencePtr subInference = currentStates[0]->getSubInference();
#ifdef JUCE_DEBUG
      for (size_t i = 1; i < currentStates.size(); ++i)
        jassert(currentStates[i]->getSubInference() == subInference);
#endif // JUCE_DEBUG

      // apply sub-learner if it exists
      
      // tmp
      if (!subInference->getBatchLearner())
        subInference->setBatchLearner(simulationInferenceLearner());
      // -

      if (subInference->getBatchLearner())
      {
        ReturnCode res = context->train(subInference, subTrainingData);
        if (res != finishedReturnCode)
          return res;
      }
      
      // evaluate sub-inference and update currentObjects
      InferencePtr evaluateStepOnSubTrainingData = new RunSequentialInferenceStepOnExamples(inference, currentStates);
      context->runInference(evaluateStepOnSubTrainingData, subTrainingData, ObjectPtr(), returnCode);

      // check if we have reached the final state on all examples
      bool areAllStatesFinal = true;
      for (size_t i = 0; i < currentStates.size(); ++i)
        if (!currentStates[i]->isFinal())
        {
          areAllStatesFinal = false;
          break;
        }
      if (areAllStatesFinal)
        return finishedReturnCode;

      ++stepNumber;
    }
  }

protected:
  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    SequentialInferencePtr inference = input[0].getObjectAndCast<SequentialInference>();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();
    jassert(inference && trainingData);
    returnCode = train(context, inference, trainingData);
    return ObjectPtr();
  }
};*/

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_STATIC_SEQUENTIAL_LEARNER_H_
