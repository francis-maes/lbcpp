/*-----------------------------------------.---------------------------------.
| Filename: Inference.cpp                  | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/Pair.h>
#include <lbcpp/Execution/ExecutionStack.h>
#include <lbcpp/Inference/Inference.h>
#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/InferenceCallback.h>
#include <lbcpp/Inference/InferenceOnlineLearner.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>
using namespace lbcpp;

/*
** Inference
*/
Inference::~Inference()
{
  while (onlineLearner)
  {
    onlineLearner->setPreviousLearner(InferenceOnlineLearnerPtr());
    onlineLearner = onlineLearner->getNextLearner();
  }
}

String Inference::getDescription(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {return getClassName() + T("(") + input.toShortString() + T(", ") + supervision.toShortString() + T(")");}

void Inference::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const InferencePtr& target = t.staticCast<Inference>();
  Function::clone(context, target);
  if (onlineLearner)
  {
    target->onlineLearner = onlineLearner->cloneAndCast<InferenceOnlineLearner>(context);
    jassert(!target->onlineLearner->getNextLearner() || target->onlineLearner->getNextLearner()->getPreviousLearner() == target->onlineLearner);
  }
  ScopedReadLock _(parametersLock);
  if (parameters.exists())
  {
    target->parameters = getParametersCopy(context); // clone parameters
    target->parametersChangedCallback();
  }
}

/*
** Parameters
*/
Variable Inference::getParameters() const
{
  ScopedReadLock _(parametersLock);
  return parameters;
}

Variable Inference::getParametersCopy(ExecutionContext& context) const
{
  ScopedReadLock _(parametersLock);
  return parameters.isObject() ? Variable(parameters.getObject()->clone(context)) : parameters;
}

void Inference::setParameters(const Variable& parameters)
{
  if (checkInheritance(parameters, getParametersType()))
  {
    ScopedWriteLock _(parametersLock);
    this->parameters = parameters;
  }
  parametersChangedCallback();
}

/*
** Learners
*/
void Inference::setBatchLearner(InferencePtr batchLearner)
{
  if (batchLearner->getName() == T("Unnamed"))
    batchLearner->setName(getName() + T(" learner"));
  this->batchLearner = batchLearner;
}

void Inference::addOnlineLearner(const InferenceOnlineLearnerPtr& learner, bool insertInFront)
{
  InferenceOnlineLearnerPtr currentLearner = this->onlineLearner;
  if (!currentLearner)
    this->onlineLearner = learner;
  else if (insertInFront)
  {
    this->onlineLearner = learner;
    if (currentLearner)
      learner->setNextLearner(currentLearner);
  }
  else
    currentLearner->getLastLearner()->setNextLearner(learner);
}

const InferenceOnlineLearnerPtr& Inference::getOnlineLearner() const
  {return onlineLearner;}

InferenceOnlineLearnerPtr Inference::getLastOnlineLearner() const
  {return onlineLearner->getLastLearner();}

/*
** High Level functions
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

bool Inference::run(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {return internalRunInference(context, refCountedPointerFromThis(this), input, supervision, NULL);}

bool Inference::run(ExecutionContext& context, const Variable& input, const Variable& supervision, Variable& output) const
  {return internalRunInference(context, refCountedPointerFromThis(this), input, supervision, &output);}

bool Inference::train(ExecutionContext& context, ContainerPtr trainingExamples, ContainerPtr validationExamples)
  {return train(context, new InferenceBatchLearnerInput(refCountedPointerFromThis(this), trainingExamples, validationExamples));}

bool Inference::train(ExecutionContext& context, const InferenceBatchLearnerInputPtr& learnerInput)
  {return batchLearner && batchLearner->run(context, learnerInput, Variable());}

bool Inference::evaluate(ExecutionContext& context, ContainerPtr examples, EvaluatorPtr evaluator) const
{
  InferencePtr inference = refCountedPointerFromThis(this);
  InferenceCallbackPtr evaluationCallback = evaluationInferenceCallback(inference, evaluator);
  context.appendCallback(evaluationCallback);
  bool res = runOnSupervisedExamplesInference(inference, true)->run(context, examples, Variable());
  context.removeCallback(evaluationCallback);
  return res;
}

bool Inference::crossValidate(ExecutionContext& context, ContainerPtr examples, EvaluatorPtr evaluator, size_t numFolds) const
{
  InferencePtr cvInference(crossValidationInference(String((int)numFolds) + T("-CV"), evaluator, refCountedPointerFromThis(this), numFolds));
  return cvInference->run(context, examples, Variable());
}

/*
** InferenceVisitor
*/
class InferenceVisitor
{
public:
  InferenceVisitor(ExecutionContext& context)
    : context(context) {}

  virtual ~InferenceVisitor() {}

  virtual void visit(InferencePtr inference)
    {accept(inference);}

protected:
  ExecutionContext& context;

  void accept(InferencePtr inference)
  {
    size_t n = inference->getNumVariables();
    for (size_t i = 0; i < n; ++i)
    {
      if (inference->getVariableType(i)->inheritsFrom(inferenceClass))
      {
        InferencePtr subInference = inference->getVariable(i).getObjectAndCast<Inference>(context);
        if (subInference)
          visit(subInference);
      }
      else if (inference->getVariableType(i)->inheritsFrom(containerClass(inferenceClass)))
      {
        ContainerPtr subInferences = inference->getVariable(i).getObjectAndCast<Container>(context);
        size_t m = subInferences->getNumElements();
        for (size_t j = 0; j < m; ++j)
        {
          InferencePtr subInference = subInferences->getElement(j).getObjectAndCast<Inference>(context);
          if (subInference)
            visit(subInference);
        }
      }
    }
  }
};

class GetInferencesThatHaveAnOnlineLearnerVisitor : public InferenceVisitor
{
public:
  GetInferencesThatHaveAnOnlineLearnerVisitor(ExecutionContext& context, std::vector<InferencePtr>& res)
    : InferenceVisitor(context), res(res) {}

  std::vector<InferencePtr>& res;

  virtual void visit(InferencePtr inference)
  {
    accept(inference);
    if (inference->getOnlineLearner())
      res.push_back(inference);
  }
};

void Inference::getInferencesThatHaveAnOnlineLearner(ExecutionContext& context, std::vector<InferencePtr>& res) const
{
  GetInferencesThatHaveAnOnlineLearnerVisitor visitor(context, res);
  visitor.visit(refCountedPointerFromThis(this));
}
