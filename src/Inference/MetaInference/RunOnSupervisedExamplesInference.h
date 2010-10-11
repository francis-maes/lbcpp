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

namespace lbcpp
{

template<class BaseClass>
class RunOnSupervisedExamplesInference : public BaseClass
{
public:
  RunOnSupervisedExamplesInference(InferencePtr inference)
    : BaseClass(T("RunOnSupervisedExamples")), inference(inference) {}
  RunOnSupervisedExamplesInference() {}

  virtual TypePtr getInputType() const
    {return containerClass(pairClass(inference->getInputType(), inference->getSupervisionType()));}

  virtual TypePtr getSupervisionType() const
    {return nilType;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return nilType;}

  virtual String getDescription(const Variable& input, const Variable& supervision) const
  {
    ContainerPtr examples = input.getObjectAndCast<Container>();
    return T("Run ") + inference->getName() + T(" with ") + 
      String((int)examples->getNumElements()) + T(" ") + examples->getElementsType()->getName() + T("(s)");
  }

protected:
  InferencePtr inference;
};

class RunOnSupervisedExamplesSequentialInference : public RunOnSupervisedExamplesInference<SequentialInference>
{
public:
  typedef RunOnSupervisedExamplesInference<SequentialInference> BaseClass;

  RunOnSupervisedExamplesSequentialInference(InferencePtr inference)
    : BaseClass(inference) {}
  RunOnSupervisedExamplesSequentialInference() {}

  virtual SequentialInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    SequentialInferenceStatePtr res(new SequentialInferenceState(input, supervision));
    updateInference(context, res, returnCode);
    return res;
  }

  virtual bool updateInference(InferenceContextPtr context, SequentialInferenceStatePtr state, ReturnCode& returnCode)
  {
    ContainerPtr examples = state->getInput().dynamicCast<Container>();
    jassert(examples);

    int nextIndex = state->getStepNumber() + 1; 
    if (nextIndex < (int)examples->getNumElements())
    {
      Variable example = examples->getElement(nextIndex);
      state->setSubInference(inference, example[0], example[1]);
      return true;
    }
    else
      return false;
  }
};

// Input: (Input, Supervision) Container
// Supervision: None
// Output: None
class RunOnSupervisedExamplesParallelInference : public RunOnSupervisedExamplesInference<ParallelInference>
{
public:
  typedef RunOnSupervisedExamplesInference<ParallelInference> BaseClass;

  RunOnSupervisedExamplesParallelInference(InferencePtr inference)
    : BaseClass(inference) {}
  RunOnSupervisedExamplesParallelInference() {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ContainerPtr examples = input.dynamicCast<Container>();
    jassert(examples);

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    size_t n = examples->getNumElements();
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable example = examples->getElement(i);
      res->addSubInference(inference, example[0], example[1]);
    }
    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
    {return Variable();}
};

class RunSequentialInferenceStepOnExamples : public ParallelInference
{
public:
  RunSequentialInferenceStepOnExamples(SequentialInferencePtr inference, std::vector<SequentialInferenceStatePtr>& currentStates)
    : ParallelInference(T("RunSequentialInferenceStepOnExamples")), inference(inference), currentStates(currentStates) {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ContainerPtr examples = input.dynamicCast<Container>();
    jassert(examples);

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    size_t n = examples->getNumElements();
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable example = examples->getElement(i);
      res->addSubInference(currentStates[i]->getSubInference(), example[0], example[1]);
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
