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

class ParallelPerEpisodeGDOnlineLearner : public GradientDescentOnlineLearner
{
public:
  ParallelPerEpisodeGDOnlineLearner(const FunctionPtr& lossFunction, const IterationFunctionPtr& learningRate, bool normalizeLearningRate = true)
    : GradientDescentOnlineLearner(lossFunction, learningRate, normalizeLearningRate) {}

  ParallelPerEpisodeGDOnlineLearner() {}
 
  virtual void startEpisode(const ObjectPtr& inputs)
  {
    DoubleVectorPtr parameters;
    {
      ScopedLock _(parametersLock);
      parameters = function->getParameters()->cloneAndCast<DoubleVector>();
    }
    ScopedLock _(threadInfoLock);
    threadInfo[Thread::getCurrentThreadId()] = std::make_pair(parameters, function->createParameters());
  }

  virtual void learningStep(const Variable* inputs, const Variable& output)
  {
    // FIXME: there is a conceptual problem here:
    // to be multithread-safe,
    // we want to perform the prediction with the thread-owned parameters instead of the Function-owned parameters
    // in learningStep(), the output is already predicted, so it is to late...
    std::pair<DoubleVectorPtr, DoubleVectorPtr> paramsAndGradient;
    {
      ScopedLock _(threadInfoLock);
      paramsAndGradient = threadInfo[Thread::getCurrentThreadId()];
    }
    jassert(paramsAndGradient.first && paramsAndGradient.second);
    computeAndAddGradient(inputs, output, paramsAndGradient.second, 1.0);
  }

  virtual void finishEpisode(const ObjectPtr& inputs, const Variable& output)
  {
    DoubleVectorPtr episodeGradient;
    {
      ScopedLock _(threadInfoLock);
      DoubleVectorPtr& dv = threadInfo[Thread::getCurrentThreadId()].second;
      episodeGradient = dv;
      dv = DoubleVectorPtr();
    }
    jassert(episodeGradient);
    ScopedLock _(parametersLock);
    gradientDescentStep(episodeGradient);
  }

protected:
  std::map<Thread::ThreadID, std::pair<DoubleVectorPtr, DoubleVectorPtr> > threadInfo; // threadID -> parameters, gradient
  CriticalSection threadInfoLock;
  CriticalSection parametersLock;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_PER_EPISODE_GD_ONLINE_LEARNER_H_
