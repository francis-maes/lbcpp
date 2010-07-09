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
#include <lbcpp/Inference/InferenceStack.h>
#include <lbcpp/Object/ObjectPair.h>
#include <lbcpp/Object/Container.h>
using namespace lbcpp;

/*
** InferenceContext
*/
Variable InferenceContext::callRunInference(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {return inference->run(InferenceContextPtr(this), input, supervision, returnCode);}

Variable InferenceContext::runDecoratorInference(DecoratorInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  std::pair<Variable, Variable> inputAndSupervision = inference->prepareSubInference(input, supervision, returnCode);
  if (returnCode != Inference::finishedReturnCode)
    return Variable();
  
  InferencePtr subInference = inference->getSubInference();
  if (!subInference)
  {
    returnCode = Inference::errorReturnCode;
    return Variable();
  }
  
  Variable subOutput = runInference(subInference, inputAndSupervision.first, inputAndSupervision.second, returnCode);
  if (returnCode != Inference::finishedReturnCode)
    return Variable();
  
  return inference->finalizeSubInference(input, supervision, subOutput, returnCode);
}

Variable InferenceContext::runSequentialInference(SequentialInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  InferenceContextPtr pthis(this);

  SequentialInferenceStatePtr state = inference->prepareInference(pthis, input, supervision, returnCode);
  if (!state)
    return ObjectPtr();
  while (true)
  {
    Variable subOutput = runInference(state->getSubInference(), state->getSubInput(), state->getSubSupervision(), returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getUserVariable();

    state->setSubOutput(subOutput);
    bool res = inference->updateInference(pthis, state, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getUserVariable();

    if (!res)
    {
      state->setFinalState();
      break;
    }
  }
  return inference->finalizeInference(pthis, state, returnCode);
}

Inference::ReturnCode InferenceContext::train(InferencePtr inference, ContainerPtr examples)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferencePtr learner = inference->getBatchLearner();
  jassert(learner);
  if (!learner)
    return Inference::errorReturnCode;
  runInference(learner, Variable::pair(inference, examples), Variable(), res);
  return res;
}

#include <lbcpp/Inference/Evaluator.h>

class EvaluationInferenceCallback : public InferenceCallback
{
public:
  EvaluationInferenceCallback(EvaluatorPtr evaluator)
    : evaluator(evaluator) {}

  virtual void postInferenceCallback(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 2 && output && supervision)
      evaluator->addPrediction(output, supervision);
  }

private:
  EvaluatorPtr evaluator;
};

Inference::ReturnCode InferenceContext::evaluate(InferencePtr inference, ContainerPtr examples, EvaluatorPtr evaluator)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferenceCallbackPtr evaluationCallback = new EvaluationInferenceCallback(evaluator);
  appendCallback(evaluationCallback);
  runInference(runOnSupervisedExamplesInference(inference), examples, Variable(), res);
  removeCallback(evaluationCallback);
  return res;
}

// temp: compatibility with ObjectContainer
class ObjectPairToContainer : public Container
{
public:
  ObjectPairToContainer() {}
  ObjectPairToContainer(ObjectContainerPtr container)
    : container(container) {}

  virtual TypePtr getElementsType() const
    {return pairType(objectClass(), objectClass());}

  virtual size_t getNumVariables() const
    {return container->size();}

  virtual Variable getVariable(size_t index) const
  {
    ObjectPairPtr pair = container->getAndCast<ObjectPair>(index);
    return pair ? Variable::pair(pair->getFirst(), pair->getSecond()) : Variable();
  }
  
  virtual void setVariable(size_t index, const Variable& value)
    {jassert(false);}

private:
  ObjectContainerPtr container;
};

// tmp
ContainerPtr convertOldStyleExamplesToNewStyle(ObjectContainerPtr examples)
{
  static bool firstTime = true;
  if (firstTime)
    {LBCPP_DECLARE_CLASS(ObjectPairToContainer, Container); firstTime = false;}
  return new ObjectPairToContainer(examples);
}

Inference::ReturnCode InferenceContext::train(InferencePtr inference, ObjectContainerPtr examplesContainer)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferencePtr learner = inference->getBatchLearner();
  jassert(learner);
  if (!learner)
    return Inference::errorReturnCode;

  ContainerPtr examples = convertOldStyleExamplesToNewStyle(examplesContainer);
  runInference(learner, Variable::pair(inference, examples), Variable(), res);
  return res;
}

void InferenceContext::callPreInference(InferenceStackPtr stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->preInferenceCallback(stack, input, supervision, output, returnCode);
}

void InferenceContext::callPostInference(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
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

  virtual Variable runInference(InferencePtr inference, const Variable& in, const Variable& sup, ReturnCode& returnCode)
  {
    stack->push(inference);
    Variable input(in);
    Variable supervision(sup);
    Variable output;
    returnCode = Inference::finishedReturnCode;
    callPreInference(stack, input, supervision, output, returnCode);
    if (returnCode == Inference::errorReturnCode)
    {
      Object::warning(T("SingleThreadedInferenceContext::runInference"), T("pre-inference failed"));
      jassert(false);
      return Variable();
    }
    
    if (returnCode == Inference::canceledReturnCode)
      {jassert(output);}
    else if (!output)
      output = callRunInference(inference, input, supervision, returnCode);  

    callPostInference(stack, input, supervision, output, returnCode);
    stack->pop();
    return output;
  }

  virtual Variable runParallelInference(ParallelInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr state = inference->prepareInference(InferenceContextPtr(this), input, supervision, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return Variable();
    
    size_t n = state->getNumSubInferences();
    for (size_t i = 0; i < n; ++i)
    {
      InferencePtr subInference = state->getSubInference(i);
      if (subInference)
      {
        returnCode = Inference::finishedReturnCode;
        Variable subOutput = runInference(subInference, state->getSubInput(i), state->getSubSupervision(i), returnCode);
        if (returnCode == Inference::errorReturnCode)
        {
          Object::error("InferenceContext::runParallelInferences", "Could not finish sub inference");
          return Variable(); 
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
