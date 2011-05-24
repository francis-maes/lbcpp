/*-----------------------------------------.---------------------------------.
| Filename: BanditsSandBox.h               | Bandits Sand Box                |
| Author  : Francis Maes                   |                                 |
| Started : 24/04/2011 14:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_

# include "../Problem/DiscreteBanditDecisionProblem.h"
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>
# include <lbcpp/Sampler/Sampler.h>
# include "GPSandBox.h"

namespace lbcpp
{
  
///////////////////////////////////////////////////
//////// Bandit policies base classes /////////////
///////////////////////////////////////////////////
  
extern ClassPtr banditStatisticsClass;

class BanditStatistics : public Object
{
public:
  BanditStatistics()
    : Object(banditStatisticsClass), statistics(new ScalarVariableStatistics(T("reward")))
  {
  }
  BanditStatistics() {}

  void update(double reward)
    {statistics->push(reward);}

  size_t getPlayedCount() const
    {return (size_t)statistics->getCount();}

  double getRewardMean() const
    {return statistics->getMean();}

  double getRewardSum()const
    {return statistics->getSum();}

  double getSquaredRewardMean() const
    {return statistics->getSquaresMean();}

  double getSquaredRewardSum() const
    {return statistics->getSquaresMean() * getPlayedCount();}

  double getRewardVariance() const
    {return statistics->getVariance();}

  double getRewardStandardDeviation() const
    {return statistics->getStandardDeviation();}

  double getMinReward() const
    {return statistics->getMinimum();}

  double getMaxReward() const
    {return statistics->getMaximum();}

private:
  friend class BanditStatisticsClass;

  ScalarVariableStatisticsPtr statistics;
};

typedef ReferenceCountedObjectPtr<BanditStatistics> BanditStatisticsPtr;

class DiscreteBanditPolicy : public Object
{
public:
  virtual size_t getRequiredNumMoments() const
    {return 2;} // unit, mean

  virtual void initialize(size_t numBandits)
  {
    timeStep = 0;
    size_t numMoments = getRequiredNumMoments();
    banditStatistics.resize(numBandits);
    for (size_t i = 0; i < numBandits; ++i)
      banditStatistics[i] = new BanditStatistics(numMoments);
  }

  size_t selectNextBandit()
    {return selectBandit(timeStep, banditStatistics);}

  void updatePolicy(size_t banditNumber, double reward)
  {
    jassert(banditNumber < banditStatistics.size());
    ++timeStep;
    banditStatistics[banditNumber]->update(reward);
  }

protected:
  friend class DiscreteBanditPolicyClass;

  size_t timeStep;
  std::vector<BanditStatisticsPtr> banditStatistics;

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) = 0;
};

typedef ReferenceCountedObjectPtr<DiscreteBanditPolicy> DiscreteBanditPolicyPtr;

class IndexBasedDiscreteBanditPolicy : public DiscreteBanditPolicy
{
protected:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const = 0;

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    if (timeStep < banditStatistics.size())
      return timeStep; // play each bandit once
    return selectMaximumIndexBandit(timeStep, banditStatistics);
  }

  size_t selectMaximumIndexBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    double bestScore = -DBL_MAX;
    size_t bestBandit = 0;
    for (size_t i = 0; i < banditStatistics.size(); ++i)
    {
      double score = computeBanditScore(i, timeStep, banditStatistics);
      if (score > bestScore)
        bestBandit = i, bestScore = score;
    }
    return bestBandit;
  }
};

///////////////////////////////////////////////////
/////////// Baseline Bandit Policies //////////////
///////////////////////////////////////////////////
class UCB1DiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
protected:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    return statistics->getRewardMean() + sqrt(2 * log((double)timeStep) / statistics->getPlayedCount());
  }
};

class UCB1TunedDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
protected:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    size_t nj = statistics->getPlayedCount();
    double lnn = log((double)timeStep);
    double varianceUB = statistics->getRewardVariance() + sqrt(2 * lnn / nj);
    return statistics->getRewardMean() + sqrt((lnn / nj) * juce::jmin(0.25, varianceUB));
  }
};

class UCB1NormalDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy
{
public:
  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double qj = statistics->getSquaredRewardSum();
    double nj = (double)statistics->getPlayedCount();
    double xj2 = statistics->getRewardMean() * statistics->getRewardMean();
    double n = (double)timeStep;
    return statistics->getRewardMean() + sqrt(16.0 * ((qj - nj * xj2) / (nj - 1)) * (log(n - 1.0) / nj));
  }

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    if (timeStep < banditStatistics.size())
      return timeStep; // play each bandit once

    // if there is a machine which has been played less then 8log(n) times, then play this machine
    size_t limit = (size_t)ceil(8 * log((double)timeStep));
    for (size_t i = 0; i < banditStatistics.size(); ++i)
      if (banditStatistics[i]->getPlayedCount() < limit)
        return i;

    return selectMaximumIndexBandit(timeStep, banditStatistics);
  }
};

///////////////////////////////////////////////////
/////////// Optimized Bandit Policy ///////////////
///////////////////////////////////////////////////

class Parameterized
{
public:
  virtual ~Parameterized() {}

  virtual SamplerPtr createParametersSampler() const = 0;

  virtual void setParameters(const Variable& parameters) = 0;
  virtual Variable getParameters() const = 0;
  virtual TypePtr getParametersType() const
    {return getParameters().getType();}

  static Parameterized* get(const ObjectPtr& object)
    {return dynamic_cast<Parameterized* >(object.get());}

  static TypePtr getParametersType(const ObjectPtr& object)
    {Parameterized* p = get(object); return p ? p->getParametersType() : TypePtr();}

  static ObjectPtr cloneWithNewParameters(const ObjectPtr& object, const Variable& newParameters)
  {
    ObjectPtr res = object->clone(defaultExecutionContext());
    Parameterized* parameterized = get(res);
    jassert(parameterized);
    parameterized->setParameters(newParameters);
    return res;
  }
};

class UCB2DiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  UCB2DiscreteBanditPolicy(double alpha = 1.0) : alpha(alpha) {}

  virtual SamplerPtr createParametersSampler() const
    {return gaussianSampler(-3, 2);}

  virtual void setParameters(const Variable& parameters)
    {alpha = pow(10, parameters.getDouble());}

  virtual Variable getParameters() const
    {return log10(alpha);}

  virtual void initialize(size_t numBandits)
  {
    IndexBasedDiscreteBanditPolicy::initialize(numBandits);
    episodeCounts.clear();
    episodeCounts.resize(numBandits, 0);
    tauValues.reserve(10000);
    episodeRemainingSteps = 0;
    currentBandit = 0;
  }

protected:
  friend class UCB2DiscreteBanditPolicyClass;

  double alpha;
  std::vector<size_t> episodeCounts;
  std::vector<size_t> tauValues;
  size_t episodeRemainingSteps;
  size_t currentBandit;

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    double tauEpisodeCount = tau(episodeCounts[banditNumber]);
    double e = 2.71828183;
    return statistics->getRewardMean() + sqrt((1.0 + alpha) * log(e * timeStep / tauEpisodeCount) / (2.0 * tauEpisodeCount));
  }

  static size_t argmax(const std::vector<double>& values)
  {
    size_t res = (size_t)-1;
    double m = -DBL_MAX;
    for (size_t i = 0; i < values.size(); ++i)
    {
      double v = values[i];
      if (v > m)
        m = v, res = i;
    }
    return res;
  }


  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t numBandits = banditStatistics.size();
    if (timeStep < numBandits)
      return timeStep; // play each bandit once

    if (episodeRemainingSteps == 0)
    {
      std::vector<double> banditScores(numBandits);
      for (size_t i = 0; i < banditScores.size(); ++i)
        banditScores[i] = computeBanditScore(i, timeStep, banditStatistics);

      while (true)
      {
        currentBandit = argmax(banditScores);
        size_t& rj = episodeCounts[currentBandit];
        episodeRemainingSteps = tau(rj + 1) - tau(rj);
        ++rj;

        if (episodeRemainingSteps == 0)
          banditScores[currentBandit] = computeBanditScore(currentBandit, timeStep, banditStatistics);
        else
          break;
      }
    }

    --episodeRemainingSteps;
    return currentBandit;
  }

  size_t tau(size_t count) const
  {
    if (count >= tauValues.size())
    {
      std::vector<size_t>& v = const_cast<UCB2DiscreteBanditPolicy* >(this)->tauValues;
      v.resize(count + 1);
      v[count] = (size_t)ceil(pow(1 + alpha, (double)count));
    }
    return tauValues[count];
  }
};

class EpsilonGreedyDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  EpsilonGreedyDiscreteBanditPolicy(double c, double d)
    : c(c), d(d), random(new RandomGenerator()) {}
  EpsilonGreedyDiscreteBanditPolicy() : c(0.0), d(0.0), random(new RandomGenerator()) {}

  virtual void initialize(size_t numBandits)
  {
    IndexBasedDiscreteBanditPolicy::initialize(numBandits);
    random->setSeed(16645186);
  }

  virtual SamplerPtr createParametersSampler() const
    {return objectCompositeSampler(pairClass(doubleType, doubleType), gaussianSampler(0.5, 0.5), gaussianSampler(0.5, 0.5));}

  virtual void setParameters(const Variable& parameters)
  {
    const PairPtr& pair = parameters.getObjectAndCast<Pair>();
    c = pair->getFirst().getDouble();
    d = pair->getSecond().getDouble();
  }

  virtual Variable getParameters() const
    {return new Pair(c, d);}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
    {return banditStatistics[banditNumber]->getRewardMean();}
 
  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t numBandits = banditStatistics.size();
    if (timeStep < numBandits)
      return timeStep; // play each bandit once

    double epsilon = juce::jmin(1.0, c * numBandits / (d * d * (timeStep + 1)));
    if (random->sampleBool(epsilon))
      return random->sampleSize(numBandits);
    else
      return selectMaximumIndexBandit(timeStep, banditStatistics);
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    IndexBasedDiscreteBanditPolicy::clone(context, target);
    target.staticCast<EpsilonGreedyDiscreteBanditPolicy>()->random = random->cloneAndCast<RandomGenerator>();
  }

protected:
  friend class EpsilonGreedyDiscreteBanditPolicyClass;

  double c;
  double d;
  RandomGeneratorPtr random;
};

class UCBvDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  UCBvDiscreteBanditPolicy() : c(1.0), zeta(1.0) {}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];

    double mean = bandit->getRewardMean();
    double variance = bandit->getRewardVariance();
    double s = (double)bandit->getPlayedCount();
    double epsilon = zeta * log((double)timeStep);
    return mean + sqrt((2.0 * variance * epsilon) / s) + c * (3.0 * epsilon) / s; 
  }

  virtual SamplerPtr createParametersSampler() const
    {return objectCompositeSampler(pairClass(doubleType, doubleType), gaussianSampler(1.0), gaussianSampler(1.0));}

  virtual void setParameters(const Variable& parameters)
  {
    const PairPtr& pair = parameters.getObjectAndCast<Pair>();
    c = pair->getFirst().getDouble();
    zeta = pair->getSecond().getDouble();
  }

  virtual Variable getParameters() const
    {return new Pair(c, zeta);}

protected:
  friend class UCBvDiscreteBanditPolicyClass;

  double c;
  double zeta;
};

class WeightedUCBBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  WeightedUCBBanditPolicy(double C = 1.0)
    : C(C) {}

  virtual SamplerPtr createParametersSampler() const
    {return gaussianSampler(1.0, 1.0);}

  virtual void setParameters(const Variable& parameters)
    {C = parameters.toDouble();}

  virtual Variable getParameters() const
    {return C;}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& statistics = banditStatistics[banditNumber];
    return statistics->getRewardMean() + C * sqrt(2 * log((double)timeStep) / statistics->getPlayedCount());
  }

protected:
  friend class WeightedUCBBanditPolicyClass;

  double C;
};

extern EnumerationPtr timeDependentWeightedUCBBanditPolicyEnumerationEnumeration;

class TimeDependentWeightedUCBBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  TimeDependentWeightedUCBBanditPolicy()
    : parameters(new DenseDoubleVector(timeDependentWeightedUCBBanditPolicyEnumerationEnumeration, doubleType)) {}

  virtual SamplerPtr createParametersSampler() const
    {return independentDoubleVectorSampler(parameters->getElementsEnumeration(), gaussianSampler(0.0, 1.0));}

  virtual void setParameters(const Variable& parameters)
    {this->parameters = parameters.getObjectAndCast<DenseDoubleVector>();}

  virtual Variable getParameters() const
    {return parameters;}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];
    double a = parameters->getValue(0);
    double b = parameters->getValue(1);
    double c = parameters->getValue(2);

    double C = a * log10((double)timeStep) + b * log10((double)bandit->getPlayedCount()) + c;
    return bandit->getRewardMean() + sqrt(2 * C * log((double)timeStep) / bandit->getPlayedCount());
  }

protected:
  friend class TimeDependentWeightedUCBBanditPolicyClass;
  // score = a * log((double)timeStep) + b * statistics->getPlayedCount() + c
  DenseDoubleVectorPtr parameters;
};

class PowerFunctionParameterizedBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  PowerFunctionParameterizedBanditPolicy(size_t maxPower, bool useSparseSampler) : maxPower(maxPower), useSparseSampler(useSparseSampler)
  {
    jassert(maxPower >= 1);
    parametersEnumeration = createParametersEnumeration();
    parameters = new DenseDoubleVector(parametersEnumeration, doubleType);
  }

  PowerFunctionParameterizedBanditPolicy() : maxPower(1) {}

  virtual ObjectPtr computeGeneratedObject(ExecutionContext& context, const String& variableName)
    {return createParametersEnumeration();}

  virtual SamplerPtr createParametersSampler() const
  {
    SamplerPtr scalarSampler = gaussianSampler(0.0, 1.0);
    if (useSparseSampler)
      scalarSampler = zeroOrScalarContinuousSampler(bernoulliSampler(0.5, 0.1, 0.9), scalarSampler);
    return independentDoubleVectorSampler(parameters->getElementsEnumeration(), scalarSampler);
  }

  virtual void setParameters(const Variable& parameters)
    {this->parameters = parameters.getObjectAndCast<DenseDoubleVector>();}

  virtual Variable getParameters() const
    {return parameters;}

  EnumerationPtr createParametersEnumeration()
  {
    DefaultEnumerationPtr parametersEnumeration = new DefaultEnumeration(T("parameters"));

    static const char* names[4] = {"sqrt(log T)", "1/sqrt(T_i)", "mean(r)", "stddev(r)"};

    for (size_t i = 0; i <= maxPower; ++i)
      for (size_t j = 0; j <= maxPower; ++j)
        for (size_t k = 0; k <= maxPower; ++k)
          for (size_t l = 0; l <= maxPower; ++l)
          {
            if (i + j + k + l == 0)
              continue; // skip unit
            String name;
            if (i)
              name += String(names[0]) + (i > 1 ? T("^") + String((int)i) : String::empty) + T(" ");
            if (j)
              name += String(names[1]) + (j > 1 ? T("^") + String((int)j) : String::empty) + T(" ");
            if (k)
              name += String(names[2]) + (k > 1 ? T("^") + String((int)k) : String::empty) + T(" ");
            if (l)
              name += String(names[3]) + (l > 1 ? T("^") + String((int)l) : String::empty) + T(" ");
            name = name.trimEnd();
            parametersEnumeration->addElement(defaultExecutionContext(), name);
          }
    return parametersEnumeration;
  }

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];
    double v1 = sqrt(log((double)timeStep));
    double v2 = 1.0 / sqrt((double)bandit->getPlayedCount());
    double v3 = bandit->getRewardMean();
    double v4 = bandit->getRewardStandardDeviation();

    double res = 0.0;
    const double* parameter = parameters->getValuePointer(0);
    for (size_t i = 0; i <= maxPower; ++i)
      for (size_t j = 0; j <= maxPower; ++j)
        for (size_t k = 0; k <= maxPower; ++k)
          for (size_t l = 0; l <= maxPower; ++l)
          {
            if (i + j + k + l == 0)
              continue; // skip unit
            res += (*parameter++) * fastPow(v1, i) * fastPow(v2, j) * fastPow(v3, k) * fastPow(v4, l);
          }
    return res;
  }

  static double fastPow(double value, size_t power)
  {
    if (power == 0)
      return 1.0;
    else if (power == 1)
      return value;
    else if (power == 2)
      return value * value;
    else if (power == 3)
      return fastPow(value, 2) * value;
    else
      return pow(value, (double)power);
  }

protected:
  friend class PowerFunctionParameterizedBanditPolicyClass;

  size_t maxPower;
  bool useSparseSampler;
  EnumerationPtr parametersEnumeration;
  DenseDoubleVectorPtr parameters;
};

extern EnumerationPtr oldStyleParameterizedBanditPolicyEnumerationEnumeration;
class OldStyleParameterizedBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  OldStyleParameterizedBanditPolicy(bool useSparseSampler = true)
    : parameters(new DenseDoubleVector(oldStyleParameterizedBanditPolicyEnumerationEnumeration, doubleType)), useSparseSampler(useSparseSampler)
  {
  }

//  virtual SamplerPtr createParametersSampler() const
//    {return independentDoubleVectorSampler(parameters->getElementsEnumeration(), gaussianSampler(0.0, 1.0));}
  virtual SamplerPtr createParametersSampler() const
  {
    SamplerPtr scalarSampler = gaussianSampler(0.0, 1.0);
    if (useSparseSampler)
      scalarSampler = zeroOrScalarContinuousSampler(bernoulliSampler(0.5, 0.1, 0.9), scalarSampler);
    return independentDoubleVectorSampler(parameters->getElementsEnumeration(), scalarSampler);
  }

  virtual void setParameters(const Variable& parameters)
    {this->parameters = parameters.getObjectAndCast<DenseDoubleVector>();}

  virtual Variable getParameters() const
    {return parameters;}

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];
  /*  size_t optimalBandit;
    double bestRewardMean = -DBL_MAX;
    for (size_t i = 0; i < banditStatistics.size(); ++i)
    {
      double rewardMean = banditStatistics[i]->getRewardMean();
      if (rewardMean > bestRewardMean)
        bestRewardMean = rewardMean, optimalBandit = i;
    }*/
    
    double ts = log((double)timeStep);
    double mean = bandit->getRewardMean();// - bestRewardMean;
    double stddev = bandit->getRewardStandardDeviation();
    double conf = sqrt(log((double)timeStep) / bandit->getPlayedCount());

    double res = 0.0;
    size_t index = 0;
    
    res += mean * parameters->getValue(index++);
    res += stddev * parameters->getValue(index++);
    res += conf * parameters->getValue(index++);

    res += ts * mean * parameters->getValue(index++);
    res += ts * stddev * parameters->getValue(index++);
    res += ts * conf * parameters->getValue(index++);

    res += mean * mean * parameters->getValue(index++);
    res += mean * stddev * parameters->getValue(index++);
    res += mean * conf * parameters->getValue(index++);

    res += stddev * stddev * parameters->getValue(index++);
    res += stddev * conf * parameters->getValue(index++);
    res += conf * conf * parameters->getValue(index++);

    return res;
  }

protected:
  friend class OldStyleParameterizedBanditPolicyClass;

  DenseDoubleVectorPtr parameters;
  bool useSparseSampler;
};

extern EnumerationPtr ultimatePolicyVariablesEnumeration;

class UltimateParameterizedBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  UltimateParameterizedBanditPolicy(GPExpressionPtr indexFunction)
    : indexFunction(indexFunction) {}

  UltimateParameterizedBanditPolicy()
    : indexFunction(new BinaryGPExpression(new VariableGPExpression(2), gpSubtraction, new VariableGPExpression(0))) {}

  virtual SamplerPtr createParametersSampler() const
    {return new GPExpressionSampler(maximumEntropySampler(gpExprLabelsEnumeration), ultimatePolicyVariablesEnumeration, 1);}

  virtual void setParameters(const Variable& parameters)
    {indexFunction = parameters.getObjectAndCast<GPExpression>();}

  virtual Variable getParameters() const
    {return indexFunction;}

  virtual TypePtr getParametersType() const
    {return gpExpressionClass;}

  virtual void initialize(size_t numBandits)
  {
    IndexBasedDiscreteBanditPolicy::initialize(numBandits);
    random = new RandomGenerator();
  }

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    BanditStatisticsPtr bandit = banditStatistics[banditNumber];

    size_t Tj = bandit->getPlayedCount();
//    double lnn = log((double)timeStep);
//    double varianceUB = bandit->getRewardVariance() + sqrt(2 * lnn / Tj);

    double inputs[4];
    inputs[0] = timeStep;
    inputs[1] = Tj;
    inputs[2] = bandit->getRewardMean();
    inputs[3] = bandit->getRewardStandardDeviation();
  /*  inputs[4] = bandit->getMinReward();
    inputs[5] = bandit->getMaxReward();
    inputs[6] = sqrt(2 * lnn / Tj);
    inputs[7] = sqrt((lnn / Tj) * juce::jmin(0.25, varianceUB));*/
    return indexFunction->compute(inputs);
  }

  virtual size_t selectBandit(size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics)
  {
    size_t numBandits = banditStatistics.size();
    if (timeStep < numBandits)
      return timeStep; // play each bandit once

    std::set<size_t> argmax;
    double bestScore = -DBL_MAX;
    for (size_t i = 0; i < numBandits; ++i)
    {
      double score = computeBanditScore(i, timeStep, banditStatistics);
      if (score >= bestScore)
      {
        if (score > bestScore)
        {
          argmax.clear();
          bestScore = score;
        }
        argmax.insert(i);
      }
    }
    if (!argmax.size())
      return random->sampleSize(numBandits);

    size_t index = random->sampleSize(argmax.size());
    std::set<size_t>::const_iterator it = argmax.begin();
    while (index > 0)
    {
      ++it;
      --index;
    }
    return *it;
    //return selectMaximumIndexBandit(timeStep, banditStatistics);
  }

protected:
  friend class UltimateParameterizedBanditPolicyClass;

  RandomGeneratorPtr random;
  GPExpressionPtr indexFunction;
};

/*
** Evaluate functions
*/
class EvaluateDiscreteBanditPolicyWorkUnit : public WorkUnit
{
public:
  EvaluateDiscreteBanditPolicyWorkUnit(size_t numBandits, size_t maxTimeStep, const std::vector<DiscreteBanditStatePtr>& initialStates, const String& initialStatesDescription, const DiscreteBanditPolicyPtr& policy,  size_t numEstimationsPerBandit = 100, bool verbose = true)
    : numBandits(numBandits), maxTimeStep(maxTimeStep), initialStates(initialStates), initialStatesDescription(initialStatesDescription), policy(policy), numEstimationsPerBandit(numEstimationsPerBandit), verbose(verbose) {}

  EvaluateDiscreteBanditPolicyWorkUnit()
    : numBandits(0), maxTimeStep(0), verbose(false) {}

  virtual String toShortString() const
    {return T("Evaluate ") + policy->getClass()->getShortName() + T("(") +  policy->toShortString() + T(") on ") + initialStatesDescription;}

  virtual Variable run(ExecutionContext& context)
  {
    // compute timeSteps
    std::vector<size_t> timeSteps;
    size_t batchSize = 2;//numBandits;
    size_t timeStep = batchSize;
    size_t batchIndex = 0;
    while (timeStep < maxTimeStep)
    {
      timeSteps.push_back(timeStep);
      ++batchIndex;
      batchSize *= 2;//numBandits;
      timeStep += batchSize;
    }
    
    // initial accumulators
    std::vector<double> actualRegretVector(timeSteps.size(), 0.0);
    std::vector<double> bestMachinePlayedVector(timeSteps.size(), 0.0);

    ScalarVariableStatisticsPtr actualRegretStatistics = new ScalarVariableStatistics(T("actualRegret")); 

    // main calculation loop
    RandomGeneratorPtr random = new RandomGenerator();
//        static int globalSeed = 1664;
  //      random->setSeed((juce::uint32)globalSeed);
    //    juce::atomicIncrement(globalSeed);

    for (size_t i = 0; i < initialStates.size(); ++i)
    {
      DiscreteBanditStatePtr initialState = initialStates[i];
      std::vector<double> expectedRewards(numBandits);
      double bestReward = -DBL_MAX;
      size_t optimalBandit = 0;
      for (size_t j = 0; j < numBandits; ++j)
      {
        double er = initialState->getExpectedReward(j);
        expectedRewards[j] = er;
        if (er > bestReward)
          bestReward = er, optimalBandit = j;
      }

      for (size_t estimation = 0; estimation < numEstimationsPerBandit; ++estimation)
      {
        DiscreteBanditStatePtr state = initialState->cloneAndCast<DiscreteBanditState>();
        state->setRandomGenerator(random);
        
        policy->initialize(numBandits);

        double sumOfRewards = 0.0;
        size_t numberOfTimesOptimalIsPlayed = 0;
        timeStep = 0;
        for (size_t j = 0; j < timeSteps.size(); ++j)
        {
          size_t numTimeSteps = timeSteps[j] - (j > 0 ? timeSteps[j - 1] : 0);
          for (size_t k = 0; k < numTimeSteps; ++k, ++timeStep)
          {
            size_t action = performBanditStep(state, policy);
            sumOfRewards += expectedRewards[action];
            if (action == optimalBandit)
              ++numberOfTimesOptimalIsPlayed;
          }
          jassert(timeStep == timeSteps[j]);
   
          actualRegretVector[j] += timeStep * bestReward - sumOfRewards;
          if (timeStep <= numBandits)
            bestMachinePlayedVector[j] += 1.0 / (double)numBandits;
          else
            bestMachinePlayedVector[j] += numberOfTimesOptimalIsPlayed / (double)timeStep;
        }
        actualRegretStatistics->push(timeStep * bestReward - sumOfRewards);
      }
      if (verbose)
        context.progressCallback(new ProgressionState(i + 1, initialStates.size(), T("Problems")));
    }

    double invZ = 1.0 / (initialStates.size() * numEstimationsPerBandit);
    for (size_t i = 0; i < timeSteps.size(); ++i)
    {
      actualRegretVector[i] *= invZ;
      bestMachinePlayedVector[i] *= invZ;
      if (verbose)
      {
        size_t timeStep = timeSteps[i];
        context.enterScope(String((int)timeStep) + T(" Steps"));
        context.resultCallback(T("log10(n)"), log10((double)timeStep));
        context.resultCallback(T("bestMachinePlayed"), bestMachinePlayedVector[i] * 100.0);
        context.resultCallback(T("actualRegret"), actualRegretVector[i]);
        context.resultCallback(T("n"), timeStep);
        context.leaveScope(actualRegretVector[i]);
      }
    }

    // TMP !!!
    //context.resultCallback(T("policy"), policy);
    //context.resultCallback(T("log10(C)"), log10(policy->getVariable(0).toDouble()));
    //context.resultCallback(T("score"), actualRegretStatistics->getMean());

    return actualRegretStatistics;
  }
 
protected:
  friend class EvaluateDiscreteBanditPolicyWorkUnitClass;

  size_t numBandits;
  size_t maxTimeStep;
  std::vector<DiscreteBanditStatePtr> initialStates;
  String initialStatesDescription;
  DiscreteBanditPolicyPtr policy;
  size_t numEstimationsPerBandit;
  bool verbose;

  static size_t performBanditStep(DiscreteBanditStatePtr state, DiscreteBanditPolicyPtr policy)
  {
    size_t action = policy->selectNextBandit();
    double reward;
    state->performTransition(defaultExecutionContext(), action, reward);
    policy->updatePolicy(action, reward);
    return action;
  }
};

class EvaluateOptimizedDiscreteBanditPolicyParameters : public SimpleUnaryFunction
{
public:
  EvaluateOptimizedDiscreteBanditPolicyParameters(const DiscreteBanditPolicyPtr& policy, size_t numBandits, size_t maxTimeStep, const std::vector<DiscreteBanditStatePtr>& initialStates, size_t numEstimationsPerBandit = 100)
    : SimpleUnaryFunction(Parameterized::getParametersType(policy), doubleType), policy(policy),
      numBandits(numBandits), maxTimeStep(maxTimeStep), initialStates(initialStates), numEstimationsPerBandit(numEstimationsPerBandit), verbose(false)
  {
  }
  EvaluateOptimizedDiscreteBanditPolicyParameters() : SimpleUnaryFunction(variableType, variableType) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    DiscreteBanditPolicyPtr policy = Parameterized::cloneWithNewParameters(this->policy, input);
    WorkUnitPtr workUnit = new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, T("Training Problems"), policy, numEstimationsPerBandit, verbose);
    ScalarVariableStatisticsPtr stats = context.run(workUnit, false).getObjectAndCast<ScalarVariableStatistics>();
    //context.informationCallback(input.toShortString() + T(" -> ") + stats->toShortString());
    return stats->getMean();
    //return stats->getMaximum();
  }

protected:
  friend class EvaluateOptimizedDiscreteBanditPolicyParametersClass;

  DiscreteBanditPolicyPtr policy;
  size_t numBandits;
  size_t maxTimeStep;
  std::vector<DiscreteBanditStatePtr> initialStates;
  size_t numEstimationsPerBandit;
  bool verbose;
};

class AddRegularizerFunction : public SimpleUnaryFunction
{
public:
  AddRegularizerFunction(FunctionPtr objectiveFunction, double l0Weight, double l1Weight, double l2Weight)
    : SimpleUnaryFunction(denseDoubleVectorClass(), doubleType),
      objectiveFunction(objectiveFunction), l0Weight(l0Weight), l1Weight(l1Weight), l2Weight(l2Weight)
  {
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr res = SimpleUnaryFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
    objectiveFunction->initialize(context, inputVariables);
    return res;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const DenseDoubleVectorPtr& inputVector = input.getObjectAndCast<DenseDoubleVector>();
    DenseDoubleVectorPtr v = inputVector->cloneAndCast<DenseDoubleVector>();
    //double l2norm = v->l2norm();
    //if (l2norm)
    //  v->multiplyByScalar(1.0 / l2norm); // normalize

    double penalty = 0.0;
    if (l0Weight)
      penalty += l0Weight * (double)v->l0norm();
    //if (l1Weight)
    //  penalty += l1Weight * (double)v->l1norm();
    //if (l2Weight)
    //  penalty += l2Weight * (double)inputVector->sumOfSquares();
    return objectiveFunction->compute(context, input).getDouble() + penalty;
  }

protected:
  FunctionPtr objectiveFunction;
  double l0Weight;
  double l1Weight;
  double l2Weight;
};

/*
** Evaluation Work Unit
*/
class EvaluateDiscreteBanditPolicy : public WorkUnit
{
public:
  EvaluateDiscreteBanditPolicy() : numBandits(2), numProblems(1000) {}

  struct EvaluatePolicyOnProblemsWorkUnit : public WorkUnit
  {
  public:
    EvaluatePolicyOnProblemsWorkUnit(const String& description, size_t numBandits, size_t maxTimeStep, const std::vector<DiscreteBanditStatePtr>& states,
                     const DiscreteBanditPolicyPtr& baselinePolicy, const DiscreteBanditPolicyPtr& optimizedPolicy)
     : description(description), numBandits(numBandits), maxTimeStep(maxTimeStep), states(states), baselinePolicy(baselinePolicy), optimizedPolicy(optimizedPolicy) {}

    virtual String toShortString() const
      {return T("Evaluate ") + description;}

    virtual Variable run(ExecutionContext& context)
    {
      context.resultCallback(T("baselinePolicy"), baselinePolicy);
      context.resultCallback(T("optimizedPolicy"), optimizedPolicy);

      // global evaluation
      context.run(new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, states, T("Global evaluation"), optimizedPolicy->cloneAndCast<DiscreteBanditPolicy>(), 100, true));

      // detailed evaluation
      context.enterScope(T("Detailed evaluation"));
      double baselineScoreSum = 0.0;
      double optimizedScoreSum = 0.0;
      size_t numberOptimizedOutperforms = 0; 
      for (size_t i = 0; i < states.size(); ++i)
      {
        context.enterScope(T("Problem ") + String((int)i));
        DiscreteBanditStatePtr initialState = states[i];
        context.resultCallback(T("index"), i);
        context.resultCallback(T("problem"), initialState);
        std::vector<DiscreteBanditStatePtr> initialStates(1, initialState);

        double baselineScore = context.run(new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, T("Baseline Policy"), baselinePolicy->cloneAndCast<DiscreteBanditPolicy>(), 100, false)).toDouble();
        baselineScoreSum += baselineScore;
        double optimizedScore = context.run(new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, T("Optimized Policy"), optimizedPolicy->cloneAndCast<DiscreteBanditPolicy>(), 100, false)).toDouble();
        optimizedScoreSum += optimizedScore;

        context.resultCallback(T("baseline"), baselineScore);
        context.resultCallback(T("optimized"), optimizedScore);
        context.resultCallback(T("delta"), optimizedScore - baselineScore);
        if (optimizedScore < baselineScore)
          ++numberOptimizedOutperforms;
        context.leaveScope(new Pair(baselineScore, optimizedScore));
        context.progressCallback(new ProgressionState(i + 1, states.size(), T("Problems")));
      }

      Variable outperformPercentage(numberOptimizedOutperforms / (double)states.size(), probabilityType);
      double baselineScore = baselineScoreSum / states.size();
      double optimizedScore = optimizedScoreSum / states.size();
      PairPtr result = new Pair(new Pair(baselineScore, optimizedScore), outperformPercentage);
      context.leaveScope(result);

      context.resultCallback(T("deltaRegret"), optimizedScore - baselineScore);
      context.resultCallback(T("outperformPercentage"), outperformPercentage);
      return new Pair(optimizedScore, outperformPercentage);
    }

  protected:
    String description;
    size_t numBandits;
    size_t maxTimeStep;
    std::vector<DiscreteBanditStatePtr> states;
    DiscreteBanditPolicyPtr baselinePolicy;
    DiscreteBanditPolicyPtr optimizedPolicy;
  };

  struct EvaluatePolicyWorkUnit : public CompositeWorkUnit
  {
  public:
    EvaluatePolicyWorkUnit(const String& policyFilePath, size_t numBandits, const std::vector<DiscreteBanditStatePtr>& states, const std::vector<DiscreteBanditStatePtr>& states2,
                     const DiscreteBanditPolicyPtr& baselinePolicy, const DiscreteBanditPolicyPtr& optimizedPolicy)
      : CompositeWorkUnit(T("Evaluating policy ") + policyFilePath)
    {
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Bernoulli problems with hor=10"), numBandits, 10, states, baselinePolicy, optimizedPolicy));
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Gaussian problems with hor=10"), numBandits, 10, states2, baselinePolicy, optimizedPolicy));

      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Bernoulli problems with hor=100"), numBandits, 100, states, baselinePolicy, optimizedPolicy));
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Gaussian problems with hor=100"), numBandits, 100, states2, baselinePolicy, optimizedPolicy));

      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Bernoulli problems with hor=1000"), numBandits, 1000, states, baselinePolicy, optimizedPolicy));
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Gaussian problems with hor=1000"), numBandits, 1000, states2, baselinePolicy, optimizedPolicy));

      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Bernoulli problems with hor=10000"), numBandits, 10000, states, baselinePolicy, optimizedPolicy));
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Gaussian problems with hor=10000"), numBandits, 10000, states2, baselinePolicy, optimizedPolicy));

      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Bernoulli problems with hor=100000"), numBandits, 100000, states, baselinePolicy, optimizedPolicy));
      addWorkUnit(new EvaluatePolicyOnProblemsWorkUnit(T("Gaussian problems with hor=100000"), numBandits, 100000, states2, baselinePolicy, optimizedPolicy));

      setProgressionUnit(T("Evaluations"));
      setPushChildrenIntoStackFlag(true);
    }
  };


  virtual Variable run(ExecutionContext& context)
  {
    if (!policyFile.exists())
    {
      context.errorCallback(policyFile.getFullPathName() + T(" does not exists"));
      return Variable();
    }

    /*
    ** Create problems
    */
    ClassPtr bernoulliSamplerClass = lbcpp::getType(T("BernoulliSampler"));
    ClassPtr gaussianSamplerClass = lbcpp::getType(T("GaussianSampler"));
    ClassPtr rejectionSamplerClass = lbcpp::getType(T("RejectionSampler"));

    SamplerPtr banditSamplerSampler = objectCompositeSampler(bernoulliSamplerClass, uniformScalarSampler(0.0, 1.0));
    SamplerPtr initialStateSampler = new DiscreteBanditInitialStateSampler(banditSamplerSampler, numBandits);
    SamplerPtr banditSamplerSampler2 = objectCompositeSampler(rejectionSamplerClass,
                                                objectCompositeSampler(gaussianSamplerClass, uniformScalarSampler(), uniformScalarSampler()),
                                                constantSampler(logicalAnd(lessThanOrEqualToPredicate(1.0), greaterThanOrEqualToPredicate(0.0))));
    SamplerPtr initialStateSampler2 = new DiscreteBanditInitialStateSampler(banditSamplerSampler2, numBandits);

    RandomGeneratorPtr random = new RandomGenerator();
    std::vector<DiscreteBanditStatePtr> states(numProblems);
    for (size_t i = 0; i < numProblems; ++i)
      states[i] = initialStateSampler->sample(context, random).getObjectAndCast<DiscreteBanditState>();

    std::vector<DiscreteBanditStatePtr> states2(numProblems);
    for (size_t i = 0; i < numProblems; ++i)
      states2[i] = initialStateSampler2->sample(context, random).getObjectAndCast<DiscreteBanditState>();

    /*
    ** Create baseline policy
    */
    DiscreteBanditPolicyPtr baselinePolicy = new UCB1TunedDiscreteBanditPolicy();

    /*
    ** Find policies to evaluate
    */
    juce::OwnedArray<File> policyFiles;
    if (policyFile.isDirectory())
      policyFile.findChildFiles(policyFiles, File::findFiles, true, T("*.policy"));
    else
      policyFiles.add(new File(policyFile));

    /*
    ** Evaluate policies
    */
    String description = T("Evaluating ");
    if (policyFiles.size() == 1)
      description += T("one policy");
    else
      description += String(policyFiles.size()) + T(" policies");
    CompositeWorkUnitPtr workUnit(new CompositeWorkUnit(description));
    for (int i = 0; i < policyFiles.size(); ++i)
    {
      File policyFile = *policyFiles[i];
      String policyFilePath = context.getFilePath(policyFile);

      DiscreteBanditPolicyPtr policy = DiscreteBanditPolicy::createFromFile(context, policyFile);
      if (policy)
        workUnit->addWorkUnit(WorkUnitPtr(new EvaluatePolicyWorkUnit(policyFilePath, numBandits, states, states2, baselinePolicy, policy)));
    }
    workUnit->setPushChildrenIntoStackFlag(true);
    workUnit->setProgressionUnit(T("Policies"));
    return context.run(workUnit);
  }


private:
  friend class EvaluateDiscreteBanditPolicyClass;

  File policyFile;
  size_t numBandits;
  size_t numProblems;
};


/*
** SandBox
*/
class BanditsSandBox : public WorkUnit
{
public:
  BanditsSandBox() : numBandits(2), maxTimeStep(100000), numTrainingProblems(100), numTestingProblems(1000), l0Weight(0.0) {}
 
  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = new RandomGenerator();

    /*
    ** Make training and testing problems
    */
    ClassPtr bernoulliSamplerClass = lbcpp::getType(T("BernoulliSampler"));
    SamplerPtr initialStateSampler = new DiscreteBanditInitialStateSampler(objectCompositeSampler(bernoulliSamplerClass, uniformScalarSampler(0.0, 1.0)), numBandits);

    std::vector<DiscreteBanditStatePtr> testingStates(numTestingProblems);
    for (size_t i = 0; i < testingStates.size(); ++i)
      testingStates[i] = initialStateSampler->sample(context, random).getObjectAndCast<DiscreteBanditState>();

    std::vector<DiscreteBanditStatePtr> trainingStates(numTrainingProblems);
    for (size_t i = 0; i < trainingStates.size(); ++i)
      trainingStates[i] = initialStateSampler->sample(context, random).getObjectAndCast<DiscreteBanditState>();

    /*
    ** Compute a bunch of baseline policies
    */
    std::vector<DiscreteBanditPolicyPtr> policies;
    //for (double power = 0.0; power >= -5.0; power -= 0.1)
    //  policies.push_back(new UCB2DiscreteBanditPolicy(pow(10.0, power)));

    //policies.push_back(new UCB2DiscreteBanditPolicy(0.001));
    policies.push_back(new UCB1TunedDiscreteBanditPolicy());
    policies.push_back(new WeightedUCBBanditPolicy(0.0));
    policies.push_back(new UltimateParameterizedBanditPolicy(new VariableGPExpression(Variable(2, ultimatePolicyVariablesEnumeration))));

    //policies.push_back(new UCBvDiscreteBanditPolicy());
    //policies.push_back(new UCB1NormalDiscreteBanditPolicy());
    //policies.push_back(new EpsilonGreedyDiscreteBanditPolicy(0.05, 0.1));

    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Evaluating policies "), 2 * policies.size());
    for (size_t i = 0; i < policies.size(); ++i)
    {
      //policies[i]->saveToFile(context, context.getFile(T("Policies/") + policies[i]->toString() + T(".policy")));
      workUnit->setWorkUnit(i * 2, makeEvaluationWorkUnit(testingStates, T("Testing Problems"), policies[i], true));
      workUnit->setWorkUnit(i * 2 + 1, makeEvaluationWorkUnit(trainingStates, T("Training Problems"), policies[i]->cloneAndCast<DiscreteBanditPolicy>(), true));
    }
    workUnit->setProgressionUnit(T("Policies"));
    workUnit->setPushChildrenIntoStackFlag(true);
    context.run(workUnit);

    ultimatePolicySearch(context, trainingStates, testingStates);
    return true;


    std::vector<std::pair<DiscreteBanditPolicyPtr, String> > policiesToOptimize;
    policiesToOptimize.push_back(std::make_pair(new PowerFunctionParameterizedBanditPolicy(2, false), T("powerFunction2-dense")));
    policiesToOptimize.push_back(std::make_pair(new PowerFunctionParameterizedBanditPolicy(2, true), T("powerFunction2-sparse")));
    //policiesToOptimize.push_back(std::make_pair(new UltimateParameterizedBanditPolicy(), T("ultimate")));

    
    //policiesToOptimize.push_back(std::make_pair(new PowerFunctionParameterizedBanditPolicy(2), T("powerFunction2")));
    /*policiesToOptimize.push_back(std::make_pair(new UCBvDiscreteBanditPolicy(), T("UCBv")));
    policiesToOptimize.push_back(std::make_pair(new UCB2DiscreteBanditPolicy(), T("UCB2")));
    policiesToOptimize.push_back(std::make_pair(new WeightedUCBBanditPolicy(), T("WeightedUCB1")));
    policiesToOptimize.push_back(std::make_pair(new EpsilonGreedyDiscreteBanditPolicy(0.1, 0.1), T("epsilonGreedy")));*/

    //policiesToOptimize.push_back(std::make_pair(new OldStyleParameterizedBanditPolicy(false), T("oldStyle")));

    //policiesToOptimize.push_back(std::make_pair(new UltimateParameterizedBanditPolicy(), T("ultimate")));
    //policiesToOptimize.push_back(std::make_pair(new OldStyleParameterizedBanditPolicy(true), T("oldStyle-sparse")));

    
    //policiesToOptimize.push_back(std::make_pair(new TimeDependentWeightedUCBBanditPolicy(), T("timeDependentWeighted")));

    //policiesToOptimize.push_back(new PerTimesWeightedUCBBanditPolicy(maxTimeStep));
    //policiesToOptimize.push_back(new WeightedUCBBanditPolicy());
    //policiesToOptimize.push_back(new OldStyleParameterizedBanditPolicy());
    
    
    //policiesToOptimize.push_back(new PerTimesWeightedUCBBanditPolicy(maxTimeStep));

    //for (size_t i = 1; i < 10; ++i)
    //  policiesToOptimize.push_back(new MomentsWeightedUCBBanditPolicy(i));

    for (size_t i = 0; i < policiesToOptimize.size(); ++i)
    {
      DiscreteBanditPolicyPtr policyToOptimize = policiesToOptimize[i].first;
      const String& policyName = policiesToOptimize[i].second;

      context.enterScope(T("Optimizing ") + policyName);
      Variable bestParameters = optimizePolicy(context, policyToOptimize, trainingStates, testingStates);
      DiscreteBanditPolicyPtr optimizedPolicy = Parameterized::cloneWithNewParameters(policyToOptimize, bestParameters);
      //String outputFileName = T("Policies100iter/") + String((int)maxTimeStep) + T("/") + policyName + T(".policy");
      //context.informationCallback(T("Saving policy ") + outputFileName);
      //optimizedPolicy->saveToFile(context, context.getFile(outputFileName));

      // evaluate on train and on test
      workUnit = new CompositeWorkUnit(T("Evaluating optimized policy"), 2);
      // policy must be cloned since it may be used in multiple threads simultaneously
      workUnit->setWorkUnit(0, makeEvaluationWorkUnit(testingStates, T("Testing Problems"), optimizedPolicy->cloneAndCast<DiscreteBanditPolicy>(), true));
      workUnit->setWorkUnit(1, makeEvaluationWorkUnit(trainingStates, T("Training Problems"), optimizedPolicy->cloneAndCast<DiscreteBanditPolicy>(), true));
      workUnit->setProgressionUnit(T("Problem Sets"));
      workUnit->setPushChildrenIntoStackFlag(true);
      context.run(workUnit);

      context.leaveScope(true);
    }
    return true;
  }

protected:
  friend class BanditsSandBoxClass;

  size_t numBandits;
  size_t maxTimeStep;

  size_t numTrainingProblems;
  size_t numTestingProblems;

  double l0Weight;

//  DiscreteBanditPolicyPtr policyToOptimize;

  WorkUnitPtr makeEvaluationWorkUnit(const std::vector<DiscreteBanditStatePtr>& initialStates, const String& initialStatesDescription, const DiscreteBanditPolicyPtr& policy, bool verbose) const
    {return new EvaluateDiscreteBanditPolicyWorkUnit(numBandits, maxTimeStep, initialStates, initialStatesDescription, policy, 100, verbose);}
  
  Variable optimizePolicy(ExecutionContext& context, DiscreteBanditPolicyPtr policy, const std::vector<DiscreteBanditStatePtr>& trainingStates,
                          const std::vector<DiscreteBanditStatePtr>& testingStates, size_t numIterations = 100)
  {
    TypePtr parametersType = Parameterized::getParametersType(policy);
    jassert(parametersType);
    size_t numParameters = 0;
    EnumerationPtr enumeration = DoubleVector::getElementsEnumeration(parametersType);
    if (enumeration)
      numParameters = enumeration->getNumElements();
    else if (parametersType->inheritsFrom(doubleType))
      numParameters = 1;
    else if (parametersType->inheritsFrom(pairClass(doubleType, doubleType)))
      numParameters = 2;
    else if (parametersType->inheritsFrom(gpExpressionClass))
      numParameters = 100;
    jassert(numParameters);
    context.resultCallback(T("numParameters"), numParameters);

    //context.resultCallback(T("parametersType"), parametersType);

    // eda parameters
    size_t populationSize = numParameters * 8;
    size_t numBests = numParameters * 2;

    if (populationSize < 50)
      populationSize = 50;
    if (numBests < 10)
      numBests = 10;


    //populationSize = 100, numBests = 10;

    // optimizer state
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(Parameterized::get(policy)->createParametersSampler());

    // optimizer context
    FunctionPtr objectiveFunction = new EvaluateOptimizedDiscreteBanditPolicyParameters(policy, numBandits, maxTimeStep, trainingStates, 100);
    if (l0Weight)
      objectiveFunction = new AddRegularizerFunction(objectiveFunction, l0Weight, 0.0, 0.0);

    objectiveFunction->initialize(context, parametersType);

    FunctionPtr validationFunction = new EvaluateOptimizedDiscreteBanditPolicyParameters(policy, numBandits, maxTimeStep, testingStates);
    validationFunction->initialize(context, parametersType);

    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, objectiveFunction, validationFunction);

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests);
    //OptimizerPtr optimizer = asyncEDAOptimizer(numIterations*populationSize, populationSize, populationSize/numBests, 1, 10, populationSize);

    optimizer->compute(context, optimizerContext, optimizerState);

    // best parameters
    Variable bestParameters = optimizerState->getBestVariable();
    context.resultCallback(T("optimizedPolicy"), Parameterized::cloneWithNewParameters(policy, bestParameters));
    return bestParameters;
  }

  bool ultimatePolicySearch(ExecutionContext& context, const std::vector<DiscreteBanditStatePtr>& trainingStates, const std::vector<DiscreteBanditStatePtr>& testingStates)
  {
    FunctionPtr objective = new EvaluateOptimizedDiscreteBanditPolicyParameters(
      new UltimateParameterizedBanditPolicy(), numBandits, maxTimeStep, trainingStates);
    FunctionPtr validation = new EvaluateOptimizedDiscreteBanditPolicyParameters(
      new UltimateParameterizedBanditPolicy(), numBandits, maxTimeStep, testingStates);
    if (!objective->initialize(context, gpExpressionClass) ||
        !validation->initialize(context, gpExpressionClass))
      return false;

    DecisionProblemStatePtr state = new GPExpressionBuilderState(T("toto"), ultimatePolicyVariablesEnumeration, objective);

    size_t maxDepth = 4;

    std::vector<std::pair<GPExpressionPtr, double> > bestExpressionsPerDepth(maxDepth, std::make_pair(GPExpressionPtr(), DBL_MAX));
    recursiveExhaustiveSearch(context, state, validation, 0, bestExpressionsPerDepth);

    for (size_t i = 0; i < bestExpressionsPerDepth.size(); ++i)
      context.informationCallback(T("Best at depth ") + String((int)i + 1) + T(" ") + bestExpressionsPerDepth[i].first->toShortString()
        + T(" [") + String(bestExpressionsPerDepth[i].second) + T("]"));
    return true;
    //return breadthFirstSearch(context, ultimatePolicyVariablesEnumeration, objective, validation);
  }


  void recursiveExhaustiveSearch(ExecutionContext& context, GPExpressionBuilderStatePtr state, const FunctionPtr& validation,
                                 size_t depth, std::vector<std::pair<GPExpressionPtr, double> >& bestExpressionsPerDepth) // bestExpressionsPerDepth determines maxDepth
  {
    if (depth)
    {
      double score = state->getScore();
      jassert((depth - 1) < bestExpressionsPerDepth.size());
      std::pair<GPExpressionPtr, double>& best = bestExpressionsPerDepth[depth - 1];
      if (!best.first || score < best.second)
      {
        best = std::make_pair(state->getExpression(), state->getScore());
        double validationScore = validation->compute(context, state->getExpression()).toDouble();
        context.informationCallback(T("D = ") + String((int)depth) + T(" E = ") + best.first->toShortString() + T(" [") + String(best.second) + T(", ") + String(validationScore) + T("]"));
      }
    }

    size_t maxDepth = bestExpressionsPerDepth.size();
    if (depth < maxDepth)
    {
      ContainerPtr actions = state->getAvailableActions();
      size_t n = actions->getNumElements();
      for (size_t i = 0; i < n; ++i)
      {
        Variable action = actions->getElement(i);
        double reward;
        state->performTransition(context, action, reward);
        recursiveExhaustiveSearch(context, state, validation, depth + 1, bestExpressionsPerDepth);
        state->undoTransition(context, action);
      }
    }
  } 


  bool breadthFirstSearch(ExecutionContext& context, EnumerationPtr inputVariables, const FunctionPtr& objective, const FunctionPtr& validation)
  {
    size_t maxSearchNodes = 50;

    DecisionProblemPtr problem = new GPExpressionBuilderProblem(inputVariables, objective);
    DecisionProblemStatePtr state = new GPExpressionBuilderState(T("toto"), inputVariables, objective);

    for (size_t depth = 0; depth < 10; ++depth)
    {
      context.enterScope(T("Depth ") + String((int)depth + 1));

      SearchTreePtr searchTree = new SearchTree(problem, state, maxSearchNodes);
      
      PolicyPtr searchPolicy = bestFirstSearchPolicy(new MinDepthSearchHeuristic());

      searchTree->doSearchEpisode(context, searchPolicy, maxSearchNodes);

      context.resultCallback(T("bestReturn"), searchTree->getBestReturn());
      bool isFinished = (searchTree->getBestReturn() <= 0.0);
      if (!isFinished)
      {
        context.resultCallback(T("bestAction"), searchTree->getBestAction());
        context.resultCallback(T("bestTrajectory"), searchTree->getBestNodeTrajectory());

        double transitionReward;
        state->performTransition(context, searchTree->getBestAction(), transitionReward);
        context.resultCallback(T("newState"), state->clone(context));
        context.resultCallback(T("transitionReward"), transitionReward);

        GPExpressionBuilderStatePtr expressionBuilderState = state.dynamicCast<GPExpressionBuilderState>();
        jassert(expressionBuilderState);
        GPExpressionPtr expression = expressionBuilderState->getExpression();
        double trainScore = expressionBuilderState->getScore();
        double validationScore = validation->compute(context, expression).toDouble();

        context.resultCallback(T("expression"), expression);
        context.resultCallback(T("score"), trainScore);
        context.resultCallback(T("validationScore"), validationScore);
        context.informationCallback(T("Best Formula: ") + expression->toShortString());

        context.leaveScope(new Pair(trainScore, validationScore));
      }
      else
      {
        context.leaveScope(state);
        break;
      }
    }
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDITS_SAND_BOX_H_
