/*-----------------------------------------.---------------------------------.
| Filename: InferenceContext.cpp           | Inference Context               |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Container.h>
#include <lbcpp/Execution/ExecutionStack.h>
#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/Inference/SequentialInference.h>
#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>
using namespace lbcpp;

/*
** InferenceWorkUnit
*/
InferenceWorkUnit::InferenceWorkUnit(const String& name, InferencePtr inference, const Variable& input, const Variable& supervision, Variable& output)
  : WorkUnit(name), inference(inference), input(input), supervision(supervision), output(output)
{
}

InferenceWorkUnit::InferenceWorkUnit()
  : output(*(Variable* )0)
{
}

String InferenceWorkUnit::toShortString() const
  {return getName();}

String InferenceWorkUnit::toString() const
  {return getName();}

bool InferenceWorkUnit::run(ExecutionContext& context)
  {return runInference(context, inference, input, supervision, output);}

/*
** InferenceContext
*/
static bool internalPreInference(ExecutionContext& context, const InferencePtr& inference, Variable& input, Variable& supervision, Variable& output)
{
  context.getStack()->push(inference);
  for (size_t i = 0; i < context.getNumCallbacks(); ++i)
  {
    InferenceCallbackPtr callback = context.getCallback(i).dynamicCast<InferenceCallback>();
    if (callback)
      callback->preInferenceCallback(context, input, supervision, output);
  }
  return true;
}

static bool internalPostInference(ExecutionContext& context, const InferencePtr& inference, Variable& input, Variable& supervision, Variable& output)
{
  for (size_t i = 0; i < context.getNumCallbacks(); ++i)
  {
    InferenceCallbackPtr callback = context.getCallback(i).dynamicCast<InferenceCallback>();
    if (callback)
      callback->postInferenceCallback(context, input, supervision, output);
  }
  context.getStack()->pop();
  return true;
}

static bool internalRunInference(ExecutionContext& context, const InferencePtr& inference, const Variable& in, const Variable& sup, Variable* out)
{
  jassert(!in.isNil());
  jassert(inference);
  Variable input(in);
  Variable supervision(sup);
  Variable output;
  if (!internalPreInference(context, inference, input, supervision, output))
  {
    context.warningCallback(T("ExecutionContext::run"), T("pre-inference failed"));
    jassert(false);
    return false;
  }

  if (!output.exists())
    output = inference->computeInference(context, input, supervision);

  internalPostInference(context, inference, input, supervision, output);
  if (out)
    *out = output;
  return true;
}

bool lbcpp::runInference(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, Variable& output)
  {return internalRunInference(context, inference, input, supervision, &output);}

bool lbcpp::runInference(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision)
  {return internalRunInference(context, inference, input, supervision, NULL);}

bool lbcpp::train(ExecutionContext& context, const InferencePtr& inference, ContainerPtr trainingExamples, ContainerPtr validationExamples)
  {return lbcpp::train(context, inference, new InferenceBatchLearnerInput(inference, trainingExamples, validationExamples));}

bool lbcpp::train(ExecutionContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& learnerInput)
{
  const InferencePtr& learner = inference->getBatchLearner();
  jassert(learner);
  return learner && runInference(context, learner, learnerInput, Variable());
}

bool lbcpp::evaluate(ExecutionContext& context, const InferencePtr& inference, ContainerPtr examples, EvaluatorPtr evaluator)
{
  InferenceCallbackPtr evaluationCallback = evaluationInferenceCallback(inference, evaluator);
  context.appendCallback(evaluationCallback);
  bool res = runInference(context, runOnSupervisedExamplesInference(inference, true), examples, Variable());
  context.removeCallback(evaluationCallback);
  return res;
}

bool lbcpp::crossValidate(ExecutionContext& context, const InferencePtr& inferenceModel, ContainerPtr examples, EvaluatorPtr evaluator, size_t numFolds)
{
  InferencePtr cvInference(crossValidationInference(String((int)numFolds) + T("-CV"), evaluator, inferenceModel, numFolds));
  return runInference(context, cvInference, examples, Variable());
}

Variable lbcpp::predict(ExecutionContext& context, const InferencePtr& inference, const Variable& input)
{
  Variable output;
  return runInference(context, inference, input, Variable(), output) ? output : Variable();
}
