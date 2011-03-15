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
  PerEpisodeGDOnlineLearner(const FunctionPtr& lossFunction, const IterationFunctionPtr& learningRate, bool normalizeLearningRate = true)
    : GradientDescentOnlineLearner(lossFunction, learningRate, normalizeLearningRate) {}

  PerEpisodeGDOnlineLearner() {}
 
  virtual void startEpisode(const ObjectPtr& inputs)
    {episodeGradient = function->createParameters();}

  virtual void learningStep(const Variable* inputs, const Variable& output)
    {computeAndAddGradient(inputs, output, episodeGradient, 1.0);}

  virtual void finishEpisode(const ObjectPtr& inputs, const Variable& output)
    {gradientDescentStep(episodeGradient);}

protected:
  DoubleVectorPtr episodeGradient;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_PER_EPISODE_GD_ONLINE_LEARNER_H_
