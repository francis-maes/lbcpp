/*-----------------------------------------.---------------------------------.
| Filename: DummyInferenceLearner.h        | A batch learner that            |
| Author  : Francis Maes                   |  does nothing                   |
| Started : 14/07/2010 13:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_META_DUMMY_LEARNER_H_
# define LBCPP_INFERENCE_META_DUMMY_LEARNER_H_

# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Inference/SequentialInference.h>

namespace lbcpp
{

class DummyInferenceLearner : public AtomicInferenceLearner
{
protected:
  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual Variable learn(InferenceContextWeakPtr context, const InferencePtr& targetInference, const ContainerPtr& trainingData)
    {return Variable();}
};

class MultiPassInferenceLearner : public InferenceLearner<VectorSequentialInference>
{
public:
  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual void prepareSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
    {state->setSubInference(getSubInference(index), state->getInput(), Variable());}
};

class InitializeByCloningInferenceLearner : public AtomicInferenceLearner
{
public:
  InitializeByCloningInferenceLearner(const InferencePtr& inferenceToClone)
    : inferenceToClone(inferenceToClone) {}
  InitializeByCloningInferenceLearner() {}

  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual Variable learn(InferenceContextWeakPtr context, const InferencePtr& targetInference, const ContainerPtr& trainingData)
  {
    MessageCallback::info(T("InitializeByCloningInferenceLearner"), T("Cloning Inference ") + inferenceToClone->getName());
    inferenceToClone->clone(targetInference);
    return Variable();
  }

protected:
  friend class InitializeByCloningInferenceLearnerClass;
  InferencePtr inferenceToClone;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_DUMMY_LEARNER_H_
