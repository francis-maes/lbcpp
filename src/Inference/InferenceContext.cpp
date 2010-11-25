/*-----------------------------------------.---------------------------------.
| Filename: InferenceContext.cpp           | Inference Context               |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/Inference/SequentialInference.h>
#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/InferenceStack.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>
#include <lbcpp/Data/Container.h>
using namespace lbcpp;

InferencePtr InferenceStack::nullInference;

Variable InferenceContext::run(const InferencePtr& inference, const Variable& in, const Variable& sup, ReturnCode& returnCode)
{
  jassert(!in.isNil());
  jassert(inference);
  Variable input(in);
  Variable supervision(sup);
  Variable output;
  returnCode = Inference::finishedReturnCode;
  preInference(inference, input, supervision, output, returnCode);
  if (returnCode == Inference::errorReturnCode)
  {
    MessageCallback::warning(T("InferenceContext::run"), T("pre-inference failed"));
    jassert(false);
    return Variable();
  }

  if (returnCode == Inference::canceledReturnCode)
    {jassert(output.exists());}
  else if (!output.exists())
    output = callRunInference(inference, input, supervision, returnCode);

  postInference(inference, input, supervision, output, returnCode);

  return output;
}

Variable InferenceContext::callRunInference(const InferencePtr& inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {return inference->computeInference(*this, input, supervision, returnCode);}

Variable InferenceContext::runDecoratorInference(DecoratorInferenceWeakPtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  DecoratorInferenceStatePtr state = inference->prepareInference(*this, input, supervision, returnCode);
  jassert(state);
  if (returnCode != Inference::finishedReturnCode)
    return Variable();

  const InferencePtr& subInference = state->getSubInference();
  if (subInference)
  {
    Variable subOutput = run(subInference, state->getSubInput(), state->getSubSupervision(), returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return Variable();
    state->setSubOutput(subOutput);
  }

  return inference->finalizeInference(*this, state, returnCode);
}

Variable InferenceContext::runSequentialInference(SequentialInferenceWeakPtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  SequentialInferenceStatePtr state = inference->prepareInference(*this, input, supervision, returnCode);
  if (!state)
    return Variable();
  while (!state->isFinal())
  {
    Variable subOutput = run(state->getSubInference(), state->getSubInput(), state->getSubSupervision(), returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getInput();

    state->setSubOutput(subOutput);
    bool res = inference->updateInference(*this, state, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getInput();
    if (!res)
      state->setFinalState();
  }
  return inference->finalizeInference(*this, state, returnCode);
}

Inference::ReturnCode InferenceContext::train(InferencePtr inference, ContainerPtr trainingExamples, ContainerPtr validationExamples)
  {return train(inference, new InferenceBatchLearnerInput(inference, trainingExamples, validationExamples));}

Inference::ReturnCode InferenceContext::train(InferencePtr inference, const InferenceBatchLearnerInputPtr& learnerInput)
{
  const InferencePtr& learner = inference->getBatchLearner();
  jassert(learner);
  if (!learner)
    return Inference::errorReturnCode;
  ReturnCode res = Inference::finishedReturnCode;
  run(learner, learnerInput, Variable(), res);
  return res;
}

Inference::ReturnCode InferenceContext::evaluate(InferencePtr inference, ContainerPtr examples, EvaluatorPtr evaluator)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferenceCallbackPtr evaluationCallback = evaluationInferenceCallback(inference, evaluator);
  appendCallback(evaluationCallback);
  run(runOnSupervisedExamplesInference(inference, true), examples, Variable(), res);
  removeCallback(evaluationCallback);
  return res;
}

Inference::ReturnCode InferenceContext::crossValidate(InferencePtr inferenceModel, ContainerPtr examples, EvaluatorPtr evaluator, size_t numFolds)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferencePtr cvInference(crossValidationInference(String((int)numFolds) + T("-CV"), evaluator, inferenceModel, numFolds));
  run(cvInference, examples, Variable(), res);
  return res;
}

Variable InferenceContext::predict(InferencePtr inference, const Variable& input)
{
  ReturnCode returnCode = Inference::finishedReturnCode;
  return run(inference, input, Variable(), returnCode);
}

void InferenceContext::callPreInference(InferenceContext& context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->preInferenceCallback(context, stack, input, supervision, output, returnCode);
}

void InferenceContext::callPostInference(InferenceContext& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
{
  for (int i = (int)callbacks.size() - 1; i >= 0; --i)
    callbacks[i]->postInferenceCallback(context, stack, input, supervision, output, returnCode);
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
