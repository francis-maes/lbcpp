/*-----------------------------------------.---------------------------------.
| Filename: StochasticNumericalInferenc...h| Stochastic Learning             |
| Author  : Francis Maes                   |                                 |
| Started : 29/10/2010 18:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
# define LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_STOCHASTIC_H_

# include "../../Inference/BatchLearner/PrecomputePerceptionsInferenceLearner.h"

namespace lbcpp
{

class PrecomputePerceptionsNumericalInferenceLearner : public PrecomputePerceptionsInferenceLearner
{
public:
  PrecomputePerceptionsNumericalInferenceLearner(InferencePtr baseLearner)
    : PrecomputePerceptionsInferenceLearner(baseLearner) {}

  PrecomputePerceptionsNumericalInferenceLearner() {}

  virtual ClassPtr getTargetInferenceClass() const
    {return numericalInferenceClass;}

  virtual const PerceptionPtr& getPerception(const InferencePtr& targetInference) const
    {return targetInference.staticCast<NumericalInference>()->getPerception();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
