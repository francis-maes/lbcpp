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
    : memorySize(memorySize), sum(0.0), sumOfSquares(0.0) {}

  void push(size_t currentEpoch, double value)
  {
    sum += value;
    sumOfSquares += value * value;
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
      double x = observations.front().second;
      sum -= x;
      sumOfSquares -= x * x;
      observations.pop_front();
    }
  }

  size_t getNumObservations() const
    {return observations.size();}

  double getMean() const
    {return observations.size() ? sum / observations.size() : 0.0;}

  double getVariance() const
  {
    size_t n = observations.size();
    double mean = getMean();
    return n ? (sumOfSquares / n - mean * mean) : 0.0;
  }

private:
  size_t memorySize;
  std::deque< std::pair<size_t, double> > observations; // epoch reward
  double sum;
  double sumOfSquares;
};

class AutoStochasticGDOnlineLearner : public GradientDescentOnlineLearner
{
public:
  AutoStochasticGDOnlineLearner(const FunctionPtr& lossFunction, size_t episodeLength, size_t memorySize)
    : GradientDescentOnlineLearner(lossFunction, IterationFunctionPtr(), true), episodeLength(episodeLength), memorySize(memorySize), currentBandit(0)
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

    if (epoch % episodeLength == 0)
    {
      if (epoch > 0)
        finishBanditEpisode(target);
      startBanditEpisode(target);
    }

    double loss;
    computeAndAddGradient(inputs, output, target, -computeLearningRate() * currentAlpha, &loss); // computeLearningRate() returns the normalization constant
    currentLoss.push(loss);

    //if (epoch % 100 == 0)
     // clampParametersL2Norm(target);

    function->setParameters(target);
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
    //context->resultCallback(T("log") + String(powerBase) + T("(alpha)"), logAlphaMean.getMean());
    return res;
  }

private:
  friend class AutoStochasticGDOnlineLearnerClass;
  size_t episodeLength;
  size_t memorySize;

  std::vector<ScalarVariableRecentSparseStatistics> bandits;
  ScalarVariableMean logAlphaMean;

  size_t currentBandit;
  double currentAlpha;
  double previousLoss;
  DoubleVectorPtr parametersBackup;
  ScalarVariableMean currentLoss;

  void startBanditEpisode(const DoubleVectorPtr& parameters)
  {
    currentBandit = selectBandit();
    double p = (double)(minPower + (int)currentBandit);
    context->informationCallback(T("LogAlpha = ") + String(p) + T(", Mean Reward of this bandit: ") + String(bandits[currentBandit].getMean()));
    currentAlpha = pow((double)powerBase, p);
    parametersBackup = parameters ? parameters->cloneAndCast<DoubleVector>() : DoubleVectorPtr();
    previousLoss = currentLoss.getCount() ? currentLoss.getMean() : DBL_MAX;
    currentLoss.clear();
  }

  void finishBanditEpisode(DoubleVectorPtr& parameters)
  {
    double meanLoss = currentLoss.getMean();
    receiveReward(currentBandit, exp(-meanLoss));
    if (meanLoss > 5 * previousLoss)
    {
      context->informationCallback(T("Restore previous parameters. Old loss: ") + String(previousLoss) + T(" New loss: ") + String(meanLoss));
      parameters = parametersBackup; // restore previous parameters
    }
    else
      logAlphaMean.push(log10(currentAlpha)); // keep
  }

  size_t selectBandit()
  {
    double bestScore = -DBL_MAX;
    size_t bestBandit = 0;
  //  String info;
    for (size_t i = 0; i < bandits.size(); ++i)
    {
      ScalarVariableRecentSparseStatistics& stats = bandits[i];
      stats.removeTooOldValues(epoch);
      size_t tk = stats.getNumObservations();
      if (!tk)
        return i;

      double rk = stats.getMean();
     // info += Variable(rk).toShortString() + T(" [") + String((int)tk) + T("], ");

      // UCB1-Tuned
      double lnn = log((double)((!memorySize || epoch < memorySize) ? epoch : memorySize));
      double varianceUB = stats.getVariance() + sqrt(2 * lnn / tk);
      double score = rk + sqrt((lnn / tk) * juce::jmin(0.25, varianceUB));
      // -

      //double score = rk + 1.0 / tk;
      if (score > bestScore)
      {
        bestScore = score;
        bestBandit = i;
      }
    }
   // context->informationCallback(info);
    return bestBandit;
  }

  void receiveReward(size_t bandit, double reward)
    {jassert(bandit < bandits.size()); bandits[bandit].push(epoch, reward);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_STOCHASTIC_GD_ONLINE_LEARNER_H_
