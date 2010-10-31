/*-----------------------------------------.---------------------------------.
| Filename: StochasticNumericalInferenc...h| Stochastic Learning             |
| Author  : Francis Maes                   |                                 |
| Started : 29/10/2010 18:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
# define LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_STOCHASTIC_H_

# include "../../Inference/MetaInference/StochasticInferenceLearner.h"

namespace lbcpp
{

class StochasticNumericalInferenceLearner : public StochasticInferenceLearner
{
public:
  StochasticNumericalInferenceLearner(bool precomputePerceptions = true, bool randomizeExamples = false)
    : StochasticInferenceLearner(randomizeExamples), precomputePerceptions(precomputePerceptions) {}

  virtual ClassPtr getTargetInferenceClass() const
    {return numericalInferenceClass;}
  
  virtual InferencePtr createLearningPass(const InferencePtr& targetInference, ContainerPtr& trainingData)
  {
    const InferenceOnlineLearnerPtr& learner = targetInference->getOnlineLearner();
    const NumericalInferencePtr& numericalInference = targetInference.staticCast<NumericalInference>();
    const PerceptionPtr& perception = numericalInference->getPerception();
    jassert(learner);
    learner->startLearningCallback();

    if (precomputePerceptions)
    {
      TypePtr elementsType = pairClass(perception->getOutputType(), numericalInference->getSupervisionType());
      size_t n = trainingData->getNumElements();
      ContainerPtr precomputedTrainingData = vector(elementsType, n);
      for (size_t i = 0; i < n; ++i)
      {
        PairPtr example = trainingData->getElement(i).getObjectAndCast<Pair>();
        precomputedTrainingData->setElement(i, new Pair(elementsType, perception->compute(example->getFirst()), example->getSecond()));
      }
      trainingData = precomputedTrainingData;
    }

    // create sequential inference state
    return new StochasticPassInferenceLearner(std::vector<InferencePtr>(1, targetInference), randomizeExamples);
  }

protected:
  friend class StochasticNumericalInferenceLearnerClass;

  bool precomputePerceptions;
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
