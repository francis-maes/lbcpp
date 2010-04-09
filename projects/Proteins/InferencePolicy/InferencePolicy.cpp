/*-----------------------------------------.---------------------------------.
| Filename: InferencePolicy.cpp            | Inference policy base classes   |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "InferencePolicy.h"
#include "../InferenceStep/ParallelInferenceStep.h"
#include "../InferenceStep/SequenceInferenceStep.h"
using namespace lbcpp;

/*
** InferenceContext
*/
ObjectPtr InferenceContext::runInference(InferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {inference->run(InferenceContextPtr(this), input, supervision, returnCode);}

void InferenceContext::callStartInferences(size_t count)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->startInferencesCallback(count);
}

void InferenceContext::callFinishInferences()
{
  for (int i = (int)callbacks.size() - 1; i >= 0; --i)
    callbacks[i]->finishInferencesCallback();
}

void InferenceContext::callPreInference(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->preInferenceCallback(stack, input, supervision, returnCode);
}

void InferenceContext::callPostInference(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
{
  for (int i = (int)callbacks.size() - 1; i >= 0; --i)
    callbacks[i]->postInferenceCallback(stack, input, supervision, output, returnCode);
}

void InferenceContext::callClassification(InferenceStackPtr stack, ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureGeneratorPtr supervision, ReturnCode& returnCode)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->classificationCallback(stack, classifier, input, supervision, returnCode);
}

void InferenceContext::appendCallback(InferenceCallbackPtr callback)
  {callbacks.push_back(callback);}

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

/*
** SingleThreadedInferenceContext
*/
class SingleThreadedInferenceContext : public InferenceContext
{
public:
  SingleThreadedInferenceContext() : stack(new InferenceStack())
    {}

  virtual String getName() const
    {return getClassName();}

  virtual ReturnCode runWithSelfSupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples)
    {return runWithSupervisedExamples(inference, examples->apply(new ObjectToObjectPairFunction()));}

  virtual ReturnCode runWithUnsupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples)
    {return runWithSupervisedExamples(inference, examples->apply(new ObjectToObjectPairFunction(true, false)));}

  virtual ReturnCode runWithSupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples)
  {
    ReturnCode returnCode = InferenceStep::finishedReturnCode;
    callStartInferences(examples->size());
    if (returnCode)
      return returnCode;

    for (size_t i = 0; i < examples->size(); ++i)
    {
      ObjectPairPtr example = examples->get(i).dynamicCast<ObjectPair>();
      jassert(example);

      returnCode = InferenceStep::finishedReturnCode;
      runInference(inference, example->getFirst(), example->getSecond(), returnCode);
      if (returnCode == InferenceStep::errorReturnCode)
        return returnCode;
    }

    returnCode = InferenceStep::finishedReturnCode;
    callFinishInferences();
    return returnCode;
  }

  virtual ObjectPtr runInference(InferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    stack->push(inference);
    callPreInference(stack, input, supervision, returnCode);
    if (returnCode)
      return ObjectPtr();
    ObjectPtr output = InferenceContext::runInference(inference, input, supervision, returnCode);
    callPostInference(stack, input, supervision, output, returnCode);
    stack->pop();
    return output;
  }

  virtual ObjectPtr runParallelInferences(ParallelInferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ObjectPtr res = inference->createEmptyOutput(input);

    size_t n = inference->getNumSubInferences(input);
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr subOutput = runInference(inference->getSubInference(input, i),
                inference->getSubInput(input, i),
                supervision ? inference->getSubSupervision(supervision, i) : ObjectPtr(),
                returnCode);
      if (returnCode != InferenceStep::finishedReturnCode)
        return ObjectPtr(); 
      inference->setSubOutput(res, i, subOutput);
    }
    return res;
  }

  virtual FeatureGeneratorPtr runClassification(ClassifierPtr classifier, FeatureGeneratorPtr input, FeatureGeneratorPtr supervision, ReturnCode& returnCode)
  {
    callClassification(stack, classifier, input, supervision, returnCode);
    return classifier->predictLabel(input);
  }

private:
  InferenceStackPtr stack;
};

InferenceContextPtr lbcpp::singleThreadedInferenceContext()
  {return new SingleThreadedInferenceContext();}

#if 0
/*
** InferencePolicy
*/
InferenceStep::ReturnCode InferencePolicy::runOnSelfSupervisedExampleSet(InferenceStepPtr inference, ObjectContainerPtr examples)
  {return runOnSupervisedExampleSet(inference, examples->apply(new ObjectToObjectPairFunction()));}

InferenceStep::ReturnCode InferencePolicy::runOnSupervisedExampleSet(InferenceStepPtr inference, ObjectContainerPtr examples)
{
  ReturnCode returnCode = InferenceStep::finishedReturnCode;
  examples = supervisedExampleSetPreCallback(inference, examples, returnCode);
  if (returnCode != InferenceStep::finishedReturnCode)
    return returnCode;

  for (size_t i = 0; i < examples->size(); ++i)
  {
    ObjectPairPtr example = examples->get(i).dynamicCast<ObjectPair>();
    jassert(example);

    returnCode = InferenceStep::finishedReturnCode;
    runOnSupervisedExample(inference, example->getFirst(), example->getSecond(), returnCode);
    if (returnCode == InferenceStep::errorReturnCode)
      return returnCode;
  }

  returnCode = InferenceStep::finishedReturnCode;
  supervisedExampleSetPostCallback(inference, examples, returnCode);
  return returnCode;
}

ObjectPtr InferencePolicy::runOnSupervisedExample(InferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  input = supervisedExamplePreCallback(inference, input, supervision, returnCode);
  if (returnCode != InferenceStep::finishedReturnCode)
    return ObjectPtr();
  ObjectPtr output = inference->run(InferencePolicyPtr(this), input, supervision, returnCode);
  if (returnCode != InferenceStep::finishedReturnCode)
    return ObjectPtr();
  return supervisedExamplePostCallback(inference, input, output, supervision, returnCode);
}

/*
** DefaultInferencePolicy
*/
ObjectPtr DefaultInferencePolicy::doSubStep(InferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {return step->run(InferencePolicyPtr(this), input, supervision, returnCode);}

ObjectPtr DefaultInferencePolicy::doParallelStep(ParallelInferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  ObjectPtr res = step->createEmptyOutput(input);

  size_t n = step->getNumSubInferences(input);
  for (size_t i = 0; i < n; ++i)
  {
    ObjectPtr subOutput = doSubStep(step->getSubInference(input, i),
              step->getSubInput(input, i),
              supervision ? step->getSubSupervision(supervision, i) : ObjectPtr(),
              returnCode);
    if (returnCode != InferenceStep::finishedReturnCode)
      return ObjectPtr(); 
    step->setSubOutput(res, i, subOutput);
  }
  return res;
}

FeatureGeneratorPtr DefaultInferencePolicy::doClassification(ClassifierPtr classifier, FeatureGeneratorPtr input,
                                                             FeatureGeneratorPtr supervision, ReturnCode& returnCode)
  {return classifier->predictLabel(input);}

#endif // 0