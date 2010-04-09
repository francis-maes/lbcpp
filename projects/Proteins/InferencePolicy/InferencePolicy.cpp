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

