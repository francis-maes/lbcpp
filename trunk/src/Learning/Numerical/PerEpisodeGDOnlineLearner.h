/*-----------------------------------------.---------------------------------.
| Filename: PerEpisodeGDOnlineLearner.h    | PerEpisode Gradient Descent     |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_PER_EPISODE_GD_ONLINE_LEARNER_H_
# define LBCPP_LEARNING_NUMERICAL_PER_EPISODE_GD_ONLINE_LEARNER_H_

# include "GradientDescentOnlineLearner.h"

namespace lbcpp
{

class PerEpisodeGDOnlineLearner : public GradientDescentOnlineLearner
{
public:
  PerEpisodeGDOnlineLearner(const IterationFunctionPtr& learningRate, bool normalizeLearningRate = true)
    : GradientDescentOnlineLearner(learningRate, normalizeLearningRate) {}

  PerEpisodeGDOnlineLearner() {}
 
  virtual void startEpisode(const FunctionPtr& f, const ObjectPtr& inputs)
  {
    const NumericalLearnableFunctionPtr& function = f.staticCast<NumericalLearnableFunction>();
    episodeGradient =  new DenseDoubleVector(function->getParametersClass());
  }

  virtual void learningStep(const FunctionPtr& f, const Variable* inputs, const Variable& output)
  {
    const NumericalLearnableFunctionPtr& function = f.staticCast<NumericalLearnableFunction>();
    computeAndAddGradient(function, inputs, output, episodeGradient, 1.0);
    GradientDescentOnlineLearner::learningStep(f, inputs, output);
  }

  virtual void finishEpisode(const FunctionPtr& f)
  {
    NumericalLearnableFunctionPtr function = f.staticCast<NumericalLearnableFunction>();
    gradientDescentStep(function, episodeGradient);
  }

protected:
  DoubleVectorPtr episodeGradient;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_PER_EPISODE_GD_ONLINE_LEARNER_H_
