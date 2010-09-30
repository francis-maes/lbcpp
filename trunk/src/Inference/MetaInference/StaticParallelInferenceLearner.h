/*-----------------------------------------.---------------------------------.
| Filename: StaticParallelInferenceLearner.h| A batch learner that           |
| Author  : Francis Maes                   |  parallely learns               |
| Started : 26/05/2010 19:08               |  it sub-inferences              |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_META_STATIC_PARALLEL_LEARNER_H_
# define LBCPP_INFERENCE_META_STATIC_PARALLEL_LEARNER_H_

# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Data/Vector.h>

namespace lbcpp
{

class StaticParallelInferenceLearner : public ParallelInference
{
public:
  virtual TypePtr getInputType() const
    {return pairType(staticParallelInferenceClass(), containerClass(pairType(anyType(), anyType())));}

  virtual TypePtr getSupervisionType() const
    {return nilType();}

  virtual TypePtr getOutputType(TypePtr ) const
    {return nilType();}

  virtual String getDescription(const Variable& input, const Variable& supervision) const
  {
    InferencePtr targetInference = input[0].getObjectAndCast<Inference>();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();
    return T("Parallel Learning of ") + targetInference->getName() + T(" with ") + 
      String((int)trainingData->getNumElements()) + T(" ") + trainingData->getElementsType()->getTemplateArgument(0)->getName() + T("(s)");
  }

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    StaticParallelInferencePtr targetInference = input[0].getObjectAndCast<StaticParallelInference>();
    size_t numSubInferences = targetInference->getNumSubInferences();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();
    size_t n = trainingData->getNumElements();

    // Compute sub-inferences for each example
    // Compute input and supervision types for each sub-inference
    std::vector<ParallelInferenceStatePtr> currentStates(n);
    for (size_t i = 0; i < currentStates.size(); ++i)
    {
      Variable example = trainingData->getElement(i);
      currentStates[i] = targetInference->prepareInference(context, example[0], example[1], returnCode);
      if (returnCode != Inference::finishedReturnCode)
        return ParallelInferenceStatePtr();
      jassert(currentStates[i]->getNumSubInferences() == numSubInferences);
    }

    // Create Learning Parallel Inference
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(numSubInferences);
    for (size_t i = 0; i < numSubInferences; ++i)
    {
      InferencePtr subInference = targetInference->getSubInference(i);
      InferencePtr subInferenceLearner = subInference->getBatchLearner();
      if (subInferenceLearner)
      {
        VectorPtr subTrainingData = vector(pairType(subInference->getInputType(), subInference->getSupervisionType()), n);
        for (size_t j = 0; j < n; ++j)
          subTrainingData->setElement(j, Variable::pair(currentStates[j]->getSubInput(i), currentStates[j]->getSubSupervision(i)));
        res->addSubInference(subInferenceLearner, Variable::pair(subInference, subTrainingData), Variable());
      }
    }
    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
    {return Variable();}
};

class ParallelVoteInferenceLearner : public StaticParallelInferenceLearner 
{
public:
  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    StaticParallelInferencePtr targetInference = input[0].getObjectAndCast<StaticParallelInference>();
    size_t numSubInferences = targetInference->getNumSubInferences();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(numSubInferences);
    for (size_t i = 0; i < numSubInferences; ++i)
    {
      InferencePtr subInference = targetInference->getSubInference(i);
      InferencePtr subInferenceLearner = subInference->getBatchLearner();
      if (subInferenceLearner)
        res->addSubInference(subInferenceLearner, Variable::pair(subInference, trainingData), Variable());
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_STATIC_PARALLEL_LEARNER_H_
