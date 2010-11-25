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

Variable InferenceContext::runInference(const InferencePtr& inference, const Variable& in, const Variable& sup, ReturnCode& returnCode)
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
    warningCallback(T("InferenceContext::run"), T("pre-inference failed"));
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

Inference::ReturnCode InferenceContext::train(InferencePtr inference, ContainerPtr trainingExamples, ContainerPtr validationExamples)
  {return train(inference, new InferenceBatchLearnerInput(inference, trainingExamples, validationExamples));}

Inference::ReturnCode InferenceContext::train(InferencePtr inference, const InferenceBatchLearnerInputPtr& learnerInput)
{
  const InferencePtr& learner = inference->getBatchLearner();
  jassert(learner);
  if (!learner)
    return Inference::errorReturnCode;
  ReturnCode res = Inference::finishedReturnCode;
  runInference(learner, learnerInput, Variable(), res);
  return res;
}

Inference::ReturnCode InferenceContext::evaluate(InferencePtr inference, ContainerPtr examples, EvaluatorPtr evaluator)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferenceCallbackPtr evaluationCallback = evaluationInferenceCallback(inference, evaluator);
  appendCallback(evaluationCallback);
  runInference(runOnSupervisedExamplesInference(inference, true), examples, Variable(), res);
  removeCallback(evaluationCallback);
  return res;
}

Inference::ReturnCode InferenceContext::crossValidate(InferencePtr inferenceModel, ContainerPtr examples, EvaluatorPtr evaluator, size_t numFolds)
{
  ReturnCode res = Inference::finishedReturnCode;
  InferencePtr cvInference(crossValidationInference(String((int)numFolds) + T("-CV"), evaluator, inferenceModel, numFolds));
  runInference(cvInference, examples, Variable(), res);
  return res;
}

Variable InferenceContext::predict(InferencePtr inference, const Variable& input)
{
  ReturnCode returnCode = Inference::finishedReturnCode;
  return runInference(inference, input, Variable(), returnCode);
}

void InferenceContext::callPreInference(InferenceContext& context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
  {
    InferenceCallbackPtr callback = callbacks[i].dynamicCast<InferenceCallback>();
    if (callback)
      callback->preInferenceCallback(context, stack, input, supervision, output, returnCode);
  }
}

void InferenceContext::callPostInference(InferenceContext& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
{
  for (int i = (int)callbacks.size() - 1; i >= 0; --i)
  {
    InferenceCallbackPtr callback = callbacks[i].dynamicCast<InferenceCallback>();
    if (callback)
     callback->postInferenceCallback(context, stack, input, supervision, output, returnCode);
  }
}
