/*-----------------------------------------.---------------------------------.
| Filename: RunOnSupervisedExamplesInfe..h | Iterates over a set of          |
| Author  : Francis Maes                   | supervised examples             |
| Started : 26/05/2010 20:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_RUN_ON_SUPERVISED_EXAMPLES_H_
# define LBCPP_INFERENCE_RUN_ON_SUPERVISED_EXAMPLES_H_

# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Inference/SequentialInference.h>
# include <lbcpp/Object/ObjectPair.h>

namespace lbcpp
{
 
class RunOnSupervisedExamplesInference : public ParallelInference
{
public:
  RunOnSupervisedExamplesInference(InferencePtr inference)
    : ParallelInference(T("RunOnSupervisedExamples")), inference(inference) {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ObjectContainerPtr examples = input.dynamicCast<ObjectContainer>();
    jassert(examples);

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(examples->size());
    for (size_t i = 0; i < examples->size(); ++i)
    {
      ObjectPairPtr example = examples->getAndCast<ObjectPair>(i);
      res->addSubInference(inference, example->getFirst(), example->getSecond());
    }
    return res;
  }

  virtual ObjectPtr finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
    {return ObjectPtr();}

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    const_cast<RunOnSupervisedExamplesInference* >(this)->examples = input.dynamicCast<ObjectContainer>();
    jassert(examples);
    return examples->size();
  }

  virtual InferencePtr getSubInference(ObjectPtr input, size_t index) const
    {return inference;}

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return getExample(index)->getFirst();}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const
    {return getExample(index)->getSecond();}

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return ObjectPtr();}

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
    {}

protected:
  InferencePtr inference;
  ObjectContainerPtr examples;

  ObjectPairPtr getExample(size_t index) const
    {return examples->getAndCast<ObjectPair>(index);}
};

class RunSequentialInferenceStepOnExamples : public RunOnSupervisedExamplesInference
{
public:
  RunSequentialInferenceStepOnExamples(SequentialInferencePtr inference, std::vector<SequentialInferenceStatePtr>& currentStates)
    : RunOnSupervisedExamplesInference(InferencePtr()), inference(inference), currentStates(currentStates) {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ObjectContainerPtr examples = input.dynamicCast<ObjectContainer>();
    jassert(examples);

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(examples->size());
    for (size_t i = 0; i < examples->size(); ++i)
    {
      ObjectPairPtr example = examples->getAndCast<ObjectPair>(i);
      res->addSubInference(currentStates[i]->getCurrentSubInference(), example->getFirst(), example->getSecond());
    }
    return res;
  }

  virtual ObjectPtr finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    for (size_t i = 0; i < state->getNumSubInferences(); ++i)
    {
      context->makeSequentialInferenceNextState(inference, currentStates[i], state->getSubOutput(i), returnCode);
      if (returnCode != finishedReturnCode)
        break;
    }
    return ObjectPtr();
  }

  virtual InferencePtr getSubInference(ObjectPtr input, size_t index) const
    {return currentStates[index]->getCurrentSubInference();}

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    ReturnCode returnCode = finishedReturnCode;
    // FIXME: use the current context
    singleThreadedInferenceContext()->makeSequentialInferenceNextState(inference, currentStates[index], subOutput, returnCode);
    jassert(returnCode == finishedReturnCode);
  }

private:
  SequentialInferencePtr inference;
  std::vector<SequentialInferenceStatePtr>& currentStates;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_RUN_ON_SUPERVISED_EXAMPLES_H_
