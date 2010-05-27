/*-----------------------------------------.---------------------------------.
| Filename: RunOnSupervisedExamplesInfe..h | Iterates over a set of          |
| Author  : Francis Maes                   | supervised examples             |
| Started : 26/05/2010 20:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_RUN_ON_SUPERVISED_EXAMPLES_H_
# define LBCPP_INFERENCE_RUN_ON_SUPERVISED_EXAMPLES_H_

# include <lbcpp/Inference/InferenceBaseClasses.h>
# include <lbcpp/Object/ObjectPair.h>

namespace lbcpp
{
 
class RunOnSupervisedExamplesInference : public SharedParallelInference
{
public:
  RunOnSupervisedExamplesInference(InferencePtr inference)
    : SharedParallelInference(T("RunOnSupervisedExamples"), inference) {}

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    const_cast<RunOnSupervisedExamplesInference* >(this)->examples = input.dynamicCast<ObjectContainer>();
    jassert(examples);
    return examples->size();
  }

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return getExample(index)->getFirst();}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const
    {return getExample(index)->getSecond();}

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return ObjectPtr();}

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
    {}

protected:
  ObjectContainerPtr examples;

  ObjectPairPtr getExample(size_t index) const
    {return examples->getAndCast<ObjectPair>(index);}
};

class RunSequentialInferenceStepOnExamples : public RunOnSupervisedExamplesInference
{
public:
  RunSequentialInferenceStepOnExamples(SequentialInferencePtr inference, size_t subInferenceNumber, std::vector<ObjectPtr>& currentObjects)
    : RunOnSupervisedExamplesInference(inference->getSubInference(subInferenceNumber)), inference(inference), subInferenceNumber(subInferenceNumber), currentObjects(currentObjects) {}

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    ObjectPairPtr example = getExample(index);
    ObjectPtr& currentObject = currentObjects[index];
    ReturnCode returnCode = finishedReturnCode;
    currentObject = inference->finalizeSubInference(example->getFirst(), example->getSecond(), subInferenceNumber, currentObject, subOutput, returnCode);
    jassert(returnCode == finishedReturnCode);
  }

private:
  SequentialInferencePtr inference;
  size_t subInferenceNumber;

  std::vector<ObjectPtr>& currentObjects;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_RUN_ON_SUPERVISED_EXAMPLES_H_
