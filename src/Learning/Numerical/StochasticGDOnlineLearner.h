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

class ScalarVariableRecentSparseStatistics : public Object
{
public:
  ScalarVariableRecentSparseStatistics(size_t memorySize)
    : memorySize(memorySize), sum(0.0) {}

  void push(size_t currentEpoch, double value)
  {
    sum += value;
    observations.push_back(std::make_pair(currentEpoch, value));
    removeTooOldValues(currentEpoch);
  }

  void removeTooOldValues(size_t currentEpoch)
  {
    if (!memorySize || currentEpoch < memorySize)
      return;
    size_t timeLimit = currentEpoch - memorySize;
    while (observations.size() && observations.front().first <= timeLimit)
    {
      sum -= observations.front().second;
      observations.pop_front();
    }
  }

  size_t getNumObservations() const
    {return observations.size();}

  double getMean() const
    {return observations.size() ? sum / observations.size() : 0.0;}

private:
  size_t memorySize;
  std::deque< std::pair<size_t, double> > observations; // epoch reward
  double sum;
};

class AutoStochasticGDOnlineLearner : public GradientDescentOnlineLearner
{
public:
  AutoStochasticGDOnlineLearner(const FunctionPtr& lossFunction, size_t memorySize)
    : GradientDescentOnlineLearner(lossFunction, IterationFunctionPtr(), true), memorySize(memorySize)
  {
    bandits.resize(maxPower - minPower + 1, ScalarVariableRecentSparseStatistics(memorySize));
  }

  AutoStochasticGDOnlineLearner() : memorySize(0) {}

  enum
  {
    powerBase = 10,
    minPower = -4,
    maxPower = 4
  };

  virtual void learningStep(const Variable* inputs, const Variable& output)
  {
    DoubleVectorPtr target = function->getParameters();

    size_t bandit = selectBandit();
    double p = (double)(minPower + (int)bandit);
    double alpha = pow((double)powerBase, p);
    logAlphaMean.push(p);

    double loss;
    computeAndAddGradient(inputs, output, target, -computeLearningRate() * alpha, loss); // computeLearningRate() returns the normalization constant
    double reward = loss > 100 ? 0.0 : exp(-loss);
    receiveReward(bandit, reward);

    //if (epoch % 100 == 0)
     // clampParametersL2Norm(target);

    function->setParameters(target);
  }
  
  void computeAndAddGradient(const Variable* inputs, const Variable& prediction, DoubleVectorPtr& target, double weight, double& exampleLossValue)
  {
    if (failure || !inputs[1].exists())
      return; // failed or no supervision

    exampleLossValue = 0.0;

    Variable lossDerivativeOrGradient;
    if (!function->computeLoss(lossFunction, inputs, prediction, exampleLossValue, lossDerivativeOrGradient))
    {
      context->errorCallback(T("Learning failed: could not compute loss gradient"));
      failure = true;
      return;
    }
    if (!target)
      target = function->createParameters();

    ++epoch;

    const DenseDoubleVectorPtr& lossGradient = lossDerivativeOrGradient.getObjectAndCast<DenseDoubleVector>();
    jassert(lossGradient);
    if (lossGradient->l2norm() * fabs(weight) > 10)
    {
      exampleLossValue = 100.0; // skip moves bigger than 10
      lossValue.push(100.0);
    }
    else
    {
      function->addGradient(lossGradient, inputs, target, weight);
      lossValue.push(exampleLossValue);
    }
  }

  void clampParametersL2Norm(const DoubleVectorPtr& parameters, double maxL2Norm = 1.0)
  {
    double l2norm = parameters->l2norm();
    if (l2norm > maxL2Norm)
      parameters->multiplyByScalar(maxL2Norm / l2norm);
  }

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    bool res = GradientDescentOnlineLearner::finishLearningIteration(iteration, objectiveValueToMinimize);
    context->resultCallback(T("log") + String(powerBase) + T("(alpha)"), logAlphaMean.getMean());
    return res;
  }

private:
  friend class AutoStochasticGDOnlineLearnerClass;
  size_t memorySize;

  std::vector<ScalarVariableRecentSparseStatistics> bandits;
  ScalarVariableMean logAlphaMean;

  size_t selectBandit()
  {
    double bestScore = -DBL_MAX;
    size_t bestBandit = 0;
    String info;
    for (size_t i = 0; i < bandits.size(); ++i)
    {
      ScalarVariableRecentSparseStatistics& stats = bandits[i];
      stats.removeTooOldValues(epoch);
      size_t tk = stats.getNumObservations();
      if (!tk)
        return i;

      double rk = stats.getMean();
      info += Variable(rk).toShortString() + T(" [") + String((int)tk) + T("], ");

      double score = rk + 1.0 / tk;
      if (score > bestScore)
      {
        bestScore = score;
        bestBandit = i;
      }
    }
    context->informationCallback(info);
    return bestBandit;
  }

  void receiveReward(size_t bandit, double reward)
    {jassert(bandit < bandits.size()); bandits[bandit].push(epoch, reward);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_STOCHASTIC_GD_ONLINE_LEARNER_H_
