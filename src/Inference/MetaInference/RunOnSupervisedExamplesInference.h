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
# include <lbcpp/Inference/InferenceBatchLearner.h>
# include <lbcpp/Core/Pair.h>

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

  virtual String getDescription(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const ContainerPtr& examples = input.getObjectAndCast<Container>(context);
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

  virtual SequentialInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    SequentialInferenceStatePtr res(new SequentialInferenceState(input, supervision));
    updateInference(context, res);
    return res;
  }

  virtual bool updateInference(ExecutionContext& context, SequentialInferenceStatePtr state) const
  {
    ContainerPtr examples = state->getInput().dynamicCast<Container>();
    jassert(examples);

    int nextIndex = state->getStepNumber() + 1; 
    if (nextIndex < (int)examples->getNumElements())
    {
      const PairPtr& example = examples->getElement(nextIndex).getObjectAndCast<Pair>(context);
      state->setSubInference(inference, example->getFirst(), example->getSecond());
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

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    ContainerPtr examples = input.dynamicCast<Container>();
    jassert(examples);

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    size_t n = examples->getNumElements();
    res->reserve(n);
    InferenceExampleVectorPtr examplesVector = examples.dynamicCast<InferenceExampleVector>();
    if (examplesVector)
    {
      for (size_t i = 0; i < n; ++i)
      {
        const std::pair<Variable, Variable>& example = examplesVector->get(i);
        res->addSubInference(inference, example.first, example.second);
      }
    }
    else
      for (size_t i = 0; i < n; ++i)
      {
        Variable example = examples->getElement(i);
        res->addSubInference(inference, example[0], example[1]);
      }
    return res;
  }

  virtual Variable finalizeInference(ExecutionContext& context, ParallelInferenceStatePtr state) const
    {return Variable();}
};

extern ClassPtr runOnSupervisedExamplesSequentialInferenceClass;
extern ClassPtr runOnSupervisedExamplesParallelInferenceClass;

inline bool isRunOnSupervisedExamplesInference(InferencePtr inference)
{
  ClassPtr cl = inference->getClass();
  return cl->inheritsFrom(runOnSupervisedExamplesSequentialInferenceClass)
      || cl->inheritsFrom(runOnSupervisedExamplesParallelInferenceClass);
}

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_RUN_ON_SUPERVISED_EXAMPLES_H_
