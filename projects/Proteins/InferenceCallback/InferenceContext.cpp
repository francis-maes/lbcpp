/*-----------------------------------------.---------------------------------.
| Filename: InferenceContext.cpp           | Inference Context               |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "InferenceContext.h"
#include "InferenceCallback.h"
#include "../InferenceStep/ParallelInferenceStep.h"
#include "../InferenceStep/SequentialInferenceStep.h"
#include "../InferenceStep/ClassificationInferenceStep.h"
using namespace lbcpp;

/*
** InferenceContext
*/
ObjectPtr InferenceContext::runInference(InferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {return inference->run(InferenceContextPtr(this), input, supervision, returnCode);}

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

void InferenceContext::callClassification(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->classificationCallback(stack, input, supervision, returnCode);
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

class DefaultInferenceContext : public InferenceContext
{
public:
  DefaultInferenceContext(InferenceFactoryPtr factory)
    : factory(factory) {}

  virtual String getName() const
    {return getClassName();}

  virtual ReturnCode runWithSelfSupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples)
    {return runWithSupervisedExamples(inference, examples->apply(new ObjectToObjectPairFunction()));}

  virtual ReturnCode runWithUnsupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples)
    {return runWithSupervisedExamples(inference, examples->apply(new ObjectToObjectPairFunction(true, false)));}

protected:
  InferenceFactoryPtr factory;
};

/*
** SingleThreadedInferenceContext
*/
class SingleThreadedInferenceContext : public DefaultInferenceContext
{
public:
  SingleThreadedInferenceContext(InferenceFactoryPtr factory)
    : DefaultInferenceContext(factory), stack(new InferenceStack()) {}

  virtual ReturnCode runWithSupervisedExamples(InferenceStepPtr inference, ObjectContainerPtr examples)
  {
    callStartInferences(examples->size());
    
    ReturnCode returnCode = InferenceStep::finishedReturnCode;
    for (size_t i = 0; i < examples->size(); ++i)
    {
      ObjectPairPtr example = examples->get(i).dynamicCast<ObjectPair>();
      jassert(example);
      runInference(inference, example->getFirst(), example->getSecond(), returnCode);
      if (returnCode == InferenceStep::errorReturnCode)
        break;
      returnCode = InferenceStep::finishedReturnCode;
    }
    callFinishInferences();
    return returnCode;
  }

  virtual ObjectPtr runInference(InferenceStepPtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    stack->push(inference);
    callPreInference(stack, input, supervision, returnCode);
    if (returnCode)
    {
      std::cerr << "Warning: pre-inference failed with code " << returnCode << std::endl;
      jassert(false);
      return ObjectPtr();
    }
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
      {
        Object::error("InferenceContext::runParallelInferences", "Could not finish sub inference");
        return ObjectPtr(); 
      }
      inference->setSubOutput(res, i, subOutput);
    }
    return res;
  }

  virtual ObjectPtr runClassification(ClassificationInferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ClassifierPtr classifier = step->getClassifier();
    if (!classifier)
    {
      if (!supervision)
      {
        returnCode = InferenceStep::errorReturnCode;
        return ObjectPtr();
      }
      FeatureGeneratorPtr correctOutput = supervision.dynamicCast<FeatureGenerator>();
      jassert(correctOutput);
      step->setClassifier(classifier = factory->createClassifier(correctOutput->getDictionary()));
    }
    callClassification(stack, input, supervision, returnCode);
    FeatureGeneratorPtr inputFeatures = input.dynamicCast<FeatureGenerator>();    
    return classifier->predictLabel(inputFeatures);
  }

private:
  InferenceStackPtr stack;
};

InferenceContextPtr lbcpp::singleThreadedInferenceContext(InferenceFactoryPtr factory)
  {return new SingleThreadedInferenceContext(factory);}
