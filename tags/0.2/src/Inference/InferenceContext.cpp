/*-----------------------------------------.---------------------------------.
| Filename: InferenceContext.cpp           | Inference Context               |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/ParameterizedInference.h>
#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/Inference/SequentialInference.h>
#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>
#include <lbcpp/Inference/InferenceStack.h>
#include <lbcpp/Object/ObjectPair.h>
using namespace lbcpp;

/*
** InferenceContext
*/
ObjectPtr InferenceContext::callRunInference(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {return inference->run(InferenceContextPtr(this), input, supervision, returnCode);}

SequentialInferenceStatePtr InferenceContext::makeSequentialInferenceInitialState(SequentialInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  SequentialInferenceStatePtr state = new SequentialInferenceState(input, supervision);

  state->setCurrentObject(inference->prepareInference(state, returnCode));
  if (returnCode != Inference::finishedReturnCode)
    return SequentialInferenceStatePtr();

  state->setCurrentSubInference(inference->getInitialSubInference(state, returnCode));
  if (returnCode != Inference::finishedReturnCode)
    return SequentialInferenceStatePtr();

  return state;
}

void InferenceContext::makeSequentialInferenceNextState(SequentialInferencePtr inference, SequentialInferenceStatePtr state, ObjectPtr subOutput, ReturnCode& returnCode)
{
  state->setCurrentObject(inference->finalizeSubInference(state, subOutput, returnCode));
  if (returnCode != Inference::finishedReturnCode)
    return;

  state->setCurrentSubInference(inference->getNextSubInference(state, returnCode));
  if (returnCode != Inference::finishedReturnCode)
    return;

  state->incrementStepNumber();
}

ObjectPtr InferenceContext::runDecoratorInference(DecoratorInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  std::pair<ObjectPtr, ObjectPtr> inputAndSupervision = inference->prepareSubInference(input, supervision, returnCode);
  if (returnCode != Inference::finishedReturnCode)
    return ObjectPtr();
  
  InferencePtr subInference = inference->getSubInference();
  if (!subInference)
  {
    returnCode = Inference::errorReturnCode;
    return ObjectPtr();
  }
  
  ObjectPtr subOutput = runInference(subInference, inputAndSupervision.first, inputAndSupervision.second, returnCode);
  if (returnCode != Inference::finishedReturnCode)
    return ObjectPtr();
  
  return inference->finalizeSubInference(input, supervision, subOutput, returnCode);
}

ObjectPtr InferenceContext::runSequentialInference(SequentialInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  SequentialInferenceStatePtr state = makeSequentialInferenceInitialState(inference, input, supervision, returnCode);
  if (!state)
    return ObjectPtr();
  while (!state->isFinal())
  {
    std::pair<ObjectPtr, ObjectPtr> currentInputAndSupervision = inference->prepareSubInference(state, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getCurrentObject();

    ObjectPtr subOutput = runInference(state->getCurrentSubInference(), currentInputAndSupervision.first, currentInputAndSupervision.second, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getCurrentObject();

    makeSequentialInferenceNextState(inference, state, subOutput, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getCurrentObject();
  }
  return inference->finalizeInference(state, returnCode);
}

Inference::ReturnCode InferenceContext::train(InferencePtr inference, ObjectContainerPtr examples)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferenceBatchLearnerPtr learner = inference->getBatchLearner();
  jassert(learner);
  if (!learner)
    return Inference::errorReturnCode;
  runInference(learner, inference, examples, res);
  return res;
}

void InferenceContext::callPreInference(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ObjectPtr& output, ReturnCode& returnCode)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->preInferenceCallback(stack, input, supervision, output, returnCode);
}

void InferenceContext::callPostInference(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
{
  for (int i = (int)callbacks.size() - 1; i >= 0; --i)
    callbacks[i]->postInferenceCallback(stack, input, supervision, output, returnCode);
}

void InferenceContext::appendCallback(InferenceCallbackPtr callback)
  {jassert(callback); callbacks.push_back(callback);}

void InferenceContext::removeCallback(InferenceCallbackPtr callback)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    if (callbacks[i] == callback)
    {
      callbacks.erase(callbacks.begin() + i);
      break;
    }
}

void InferenceContext::clearCallbacks()
  {callbacks.clear();}

class DefaultInferenceContext : public InferenceContext
{
public:
  virtual String getName() const
    {return getClassName();}
};

/*
** SingleThreadedInferenceContext
*/
class SingleThreadedInferenceContext : public DefaultInferenceContext
{
public:
  SingleThreadedInferenceContext()
    : stack(new InferenceStack()) {}

  virtual ObjectPtr runInference(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    stack->push(inference);
    ObjectPtr output;
    returnCode = Inference::finishedReturnCode;
    callPreInference(stack, input, supervision, output, returnCode);
    if (returnCode == Inference::errorReturnCode)
    {
      std::cerr << "Warning: pre-inference failed" << std::endl;
      jassert(false);
      return ObjectPtr();
    }
    
    if (returnCode == Inference::canceledReturnCode)
      {jassert(output);}
    else if (!output)
      output = callRunInference(inference, input, supervision, returnCode);  

    callPostInference(stack, input, supervision, output, returnCode);
    stack->pop();
    return output;
  }

  virtual ObjectPtr runParallelInference(ParallelInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr state = inference->prepareInference(InferenceContextPtr(this), input, supervision, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return ObjectPtr();
    
    size_t n = state->getNumSubInferences();
    for (size_t i = 0; i < n; ++i)
    {
      InferencePtr subInference = state->getSubInference(i);
      if (subInference)
      {
        returnCode = Inference::finishedReturnCode;
        ObjectPtr subOutput = runInference(subInference, state->getSubInput(i), state->getSubSupervision(i), returnCode);
        if (returnCode == Inference::errorReturnCode)
        {
          Object::error("InferenceContext::runParallelInferences", "Could not finish sub inference");
          return ObjectPtr(); 
        }
        state->setSubOutput(i, subOutput);
      }
    }
    return inference->finalizeInference(InferenceContextPtr(this), state, returnCode);
  }

private:
  InferenceStackPtr stack;
};

InferenceContextPtr lbcpp::singleThreadedInferenceContext()
  {return new SingleThreadedInferenceContext();}