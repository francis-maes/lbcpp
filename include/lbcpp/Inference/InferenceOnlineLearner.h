/*-----------------------------------------.---------------------------------.
| Filename: InferenceOnlineLearner.h       | Inference Online Learners       |
| Author  : Francis Maes                   |                                 |
| Started : 26/05/2010 17:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONLINE_LEARNER_H_
# define LBCPP_INFERENCE_ONLINE_LEARNER_H_

# include "Inference.h"
# include "../ObjectPredeclarations.h"
# include "../Utilities/RandomVariable.h"
# include "../Utilities/IterationFunction.h"

namespace lbcpp
{

class InferenceOnlineLearner : public Object
{
public:
  enum UpdateFrequency
  {
    never,
    perStep,
    perEpisode,
    perPass,
    perStepMiniBatch,
    perStepMiniBatch2 = perStepMiniBatch + 2,
    perStepMiniBatch5 = perStepMiniBatch + 5,
    perStepMiniBatch10 = perStepMiniBatch + 10,
    perStepMiniBatch20 = perStepMiniBatch + 20,
    perStepMiniBatch50 = perStepMiniBatch + 50,
    perStepMiniBatch100 = perStepMiniBatch + 100,
    perStepMiniBatch200 = perStepMiniBatch + 200,
    perStepMiniBatch500 = perStepMiniBatch + 500,
    perStepMiniBatch1000 = perStepMiniBatch + 1000,
  };

  virtual void stepFinishedCallback(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput) = 0;
  virtual void episodeFinishedCallback(InferencePtr inference) = 0;
  virtual void passFinishedCallback(InferencePtr inference) = 0;

  virtual double getCurrentLossEstimate() const = 0;
  virtual bool isLearningStopped() const
    {return false;}

  InferenceOnlineLearnerPtr addStoppingCriterion(UpdateFrequency criterionTestFrequency, StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops = true) const;

protected:
  ParameterizedInferencePtr getParameterizedInference(InferencePtr inference) const
    {return inference.staticCast<ParameterizedInference>();}
  DenseVectorPtr getParameters(InferencePtr inference) const;
};

extern InferenceOnlineLearnerPtr gradientDescentInferenceOnlineLearner(
          // randomization
          InferenceOnlineLearner::UpdateFrequency randomizationFrequency = InferenceOnlineLearner::never,
          // learning steps
          InferenceOnlineLearner::UpdateFrequency learningUpdateFrequency = InferenceOnlineLearner::perEpisode,
          IterationFunctionPtr learningRate = constantIterationFunction(1.0),
          bool normalizeLearningRate = true,
          // regularizer
          InferenceOnlineLearner::UpdateFrequency regularizerUpdateFrequency = InferenceOnlineLearner::perEpisode,
          ScalarVectorFunctionPtr regularizer = ScalarVectorFunctionPtr(),
          // stopping criterion
          InferenceOnlineLearner::UpdateFrequency criterionTestFrequency = InferenceOnlineLearner::never,
          StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr(),
          bool restoreBestParametersWhenLearningStops = false
    );

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_ONLINE_LEARNER_H_
