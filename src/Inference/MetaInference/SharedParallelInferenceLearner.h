/*-----------------------------------------.---------------------------------.
| Filename: SharedParallelInferenceLearner.h|  A batch learner to learn      |
| Author  : Francis Maes                   |  shared parellel inferences     |
| Started : 14/07/2010 14:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_META_SHARED_PARALLEL_LEARNER_H_
# define LBCPP_INFERENCE_META_SHARED_PARALLEL_LEARNER_H_

# include <lbcpp/Inference/DecoratorInference.h>
# include <lbcpp/Inference/ParallelInference.h>

namespace lbcpp
{

class SharedParallelInferenceLearner : public InferenceLearner<DecoratorInference>
{
public:
  SharedParallelInferenceLearner(bool filterUnsupervisedExamples = true)
    : filterUnsupervisedExamples(filterUnsupervisedExamples) {}

  virtual ClassPtr getTargetInferenceClass() const
    {return sharedParallelInferenceClass;}

  virtual DecoratorInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    SharedParallelInferencePtr targetInference = getInferenceAndCast<SharedParallelInference>(input);
    ContainerPtr trainingData = getTrainingData(input);
    
    DecoratorInferenceStatePtr res(new DecoratorInferenceState(input, supervision));
    
    InferencePtr targetSubInference = targetInference->getSubInference();
    InferencePtr subLearner = targetSubInference->getBatchLearner();
    if (subLearner)
    {
      ContainerPtr subTrainingData = computeSubTrainingData(context, targetInference, trainingData, returnCode);
      if (returnCode != finishedReturnCode)
        return DecoratorInferenceStatePtr();
      res->setSubInference(subLearner, Variable::pair(targetSubInference, subTrainingData), Variable());
    }
    return res;
  }

private:
  bool filterUnsupervisedExamples;

  ContainerPtr computeSubTrainingData(InferenceContextPtr context, SharedParallelInferencePtr targetInference, ContainerPtr trainingData, ReturnCode& returnCode)
  {
    InferencePtr targetSubInference = targetInference->getSubInference();
    TypePtr pairType = pairClass(targetSubInference->getInputType(), targetSubInference->getSupervisionType());
    VectorPtr res = vector(pairType);
    
    size_t n = trainingData->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable inputAndSupervision = trainingData->getElement(i);
      ParallelInferenceStatePtr state = targetInference->prepareInference(context, inputAndSupervision[0], inputAndSupervision[1], returnCode);
      if (returnCode != finishedReturnCode)
        return ContainerPtr();
      for (size_t j = 0; j < state->getNumSubInferences(); ++j)
      {
        jassert(state->getSubInference(j) == targetSubInference);
        Variable subSupervision = state->getSubSupervision(j);
        if (!filterUnsupervisedExamples || subSupervision.exists())
          res->append(Variable::pair(state->getSubInput(j), subSupervision, pairType));
      }
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_SHARED_PARALLEL_LEARNER_H_
