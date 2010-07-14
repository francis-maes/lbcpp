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

namespace lbcpp
{

class SharedParallelInferenceLearner : public DecoratorInference
{
public:
  virtual TypePtr getInputType() const
    {return pairType(sharedParallelInferenceClass(), containerClass());}

  virtual TypePtr getSupervisionType() const
    {return nilType();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return nilType();}

  virtual DecoratorInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    SharedParallelInferencePtr targetInference = input[0].getObjectAndCast<SharedParallelInference>();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();
    
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
  ContainerPtr computeSubTrainingData(InferenceContextPtr context, SharedParallelInferencePtr targetInference, ContainerPtr trainingData, ReturnCode& returnCode)
  {
    InferencePtr targetSubInference = targetInference->getSubInference();
    VectorPtr res = new Vector(pairType(targetSubInference->getInputType(), targetSubInference->getSupervisionType()));
    
    size_t n = trainingData->size();
    for (size_t i = 0; i < n; ++i)
    {
      Variable inputAndSupervision = trainingData->getVariable(i);
      ParallelInferenceStatePtr state = targetInference->prepareInference(context, inputAndSupervision[0], inputAndSupervision[1], returnCode);
      if (returnCode != finishedReturnCode)
        return ContainerPtr();
      for (size_t j = 0; j < state->getNumSubInferences(); ++j)
      {
        jassert(state->getSubInference(j) == targetSubInference);
        res->append(Variable::pair(state->getSubInput(j), state->getSubSupervision(j)));
      }
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_SHARED_PARALLEL_LEARNER_H_
