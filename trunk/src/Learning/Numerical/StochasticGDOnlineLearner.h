/*-----------------------------------------.---------------------------------.
| Filename: StochasticGDOnlineLearner.h    | Stochastic Gradient Descent     |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_STOCHASTIC_GD_ONLINE_LEARNER_H_
# define LBCPP_LEARNING_NUMERICAL_STOCHASTIC_GD_ONLINE_LEARNER_H_

# include "GradientDescentOnlineLearner.h"

namespace lbcpp
{

class StochasticGDOnlineLearner : public GradientDescentOnlineLearner
{
public:
  StochasticGDOnlineLearner(const FunctionPtr& lossFunction, const IterationFunctionPtr& learningRate, bool normalizeLearningRate)
    : GradientDescentOnlineLearner(lossFunction, learningRate, normalizeLearningRate) {}
  StochasticGDOnlineLearner() {}

  virtual void learningStep(const Variable* inputs, const Variable& output)
  {
    DoubleVectorPtr target = function->getParameters();
    computeAndAddGradient(inputs, output, target, -computeLearningRate());
    function->setParameters(target);
  }
};


class AutoStochasticGDOnlineLearner : public StochasticGDOnlineLearner
{
public:
  AutoStochasticGDOnlineLearner(const FunctionPtr& lossFunction, size_t memorySize)
    : StochasticGDOnlineLearner(lossFunction, IterationFunctionPtr(), true), memorySize(memorySize) {}
  AutoStochasticGDOnlineLearner() : memorySize(0) {}

  virtual double computeLearningRate() const
  {
    double res = StochasticGDOnlineLearner::computeLearningRate();
    double alpha = 1.0; // FIXME
    return res * alpha;
  }

private:
  friend class AutoStochasticGDOnlineLearnerClass;
  size_t memorySize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_STOCHASTIC_GD_ONLINE_LEARNER_H_
