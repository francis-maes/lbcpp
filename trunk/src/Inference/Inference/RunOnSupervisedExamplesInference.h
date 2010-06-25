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
 
// Input: (Input, Supervision) ObjectContainer
// Supervision: None
// Output: None
class RunOnSupervisedExamplesInference : public ParallelInference
{
public:
  RunOnSupervisedExamplesInference(InferencePtr inference)
    : ParallelInference(T("RunOnSupervisedExamples")), inference(inference) {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
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

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
    {return Variable();}

protected:
  InferencePtr inference;
};

class RunSequentialInferenceStepOnExamples : public ParallelInference
{
public:
  RunSequentialInferenceStepOnExamples(SequentialInferencePtr inference, std::vector<SequentialInferenceStatePtr>& currentStates)
    : ParallelInference(T("RunSequentialInferenceStepOnExamples")), inference(inference), currentStates(currentStates) {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ObjectContainerPtr examples = input.dynamicCast<ObjectContainer>();
    jassert(examples);

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(examples->size());
    for (size_t i = 0; i < examples->size(); ++i)
    {
      ObjectPairPtr example = examples->getAndCast<ObjectPair>(i);
      res->addSubInference(currentStates[i]->getSubInference(), example->getFirst(), example->getSecond());
    }
    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    for (size_t i = 0; i < state->getNumSubInferences(); ++i)
    {
      currentStates[i]->setSubOutput(state->getSubOutput(i));
      if (!inference->updateInference(context, currentStates[i], returnCode))
        currentStates[i]->setFinalState();
      if (returnCode != finishedReturnCode)
        break;
    }
    return Variable();
  }

private:
  SequentialInferencePtr inference;
  std::vector<SequentialInferenceStatePtr>& currentStates;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_RUN_ON_SUPERVISED_EXAMPLES_H_
