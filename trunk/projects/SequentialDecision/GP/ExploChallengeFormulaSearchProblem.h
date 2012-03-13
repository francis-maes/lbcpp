/*-----------------------------------------.---------------------------------.
| Filename: ExploChallengeFormulaSearchProblem.h | Exploration Exploitation  |
| Author  : Francis Maes                   | Challenge 3                     |
| Started : 13/03/2012 13:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GP_SEARCH_PROBLEM_EXPLO_CHALLENGE_FORMULA_H_
# define LBCPP_GP_SEARCH_PROBLEM_EXPLO_CHALLENGE_FORMULA_H_

# include "FormulaSearchProblem.h"
# include "../Bandits/DiscreteBanditPolicy.h"
# include <algorithm>

namespace lbcpp
{

extern EnumerationPtr exploChallengeFormulaVariablesEnumeration;
extern EnumerationPtr exploChallengeFormulaObjectiveParametersEnumeration;

class ExploChallengeFormulaObjective : public SimpleUnaryFunction
{
public:
  ExploChallengeFormulaObjective(size_t horizon = 307000, size_t totalNumArms = 246,
                                  size_t numArmsPerRound = 30, size_t numDocumentsAliveSimultaneously = 80,
                                  double minRewardExpectation = 0.0, double maxRewardExpectation = 0.05)
    : SimpleUnaryFunction(gpExpressionClass, doubleType), horizon(horizon),
      totalNumArms(totalNumArms), numArmsPerRound(numArmsPerRound), numDocumentsAliveSimultaneously(numDocumentsAliveSimultaneously),
      minRewardExpectation(minRewardExpectation), maxRewardExpectation(maxRewardExpectation)
  {

  }

  struct ArmInfo
  {
    ArmInfo() : presentedCount(0) {}

    size_t presentedCount;
    ScalarVariableStatistics stats;
  };

  static double sampleGamma(RandomGeneratorPtr random, double k, double theta)
  {
    if (k < 1)
    {
      // Weibull algorithm
      double c = (1 / k);
      double d = ((1 - k) * pow(k, (k / (1 - k))));
      double u, v, z, e, x;
      while (true)
      {
        u = random->sampleDouble();
        v = random->sampleDouble();
        z = -log(u);
        e = -log(v);
        x = pow(z, c);
        if ((z + e) >= (d + x))
         break;
      }
      return x * theta;
    }
    else
    {
      // Cheng's algorithm
      double b = (k - log(4.0));
      double c = (k + sqrt(2.0 * k - 1.0));
      double lam = sqrt(2 * k - 1.0);
      double cheng = (1 + log(4.5));
      double u, v, x, y, z, r;
      while (true)
      {
        u = random->sampleDouble();
        v = random->sampleDouble();
        y = ((1 / lam) * log(v / (1 - v)));
        x = (k * exp(y));
        z = (u * v * v);
        r = (b + (c * y) - x);
        if ((r >= ((4.5 * z) - cheng)) || (r >= log(z)))
          break;
      }
      return x * theta;
    }
  }

/*  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    RandomGeneratorPtr random = 
  }*/

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    GPExpressionPtr formula = input.getObjectAndCast<GPExpression>();

    // initialize arms
    std::vector<SamplerPtr> arms(totalNumArms);
    for (size_t i = 0; i < totalNumArms; ++i)
    {
      //double expectation = sampleGamma(random, minRewardExpectation, maxRewardExpectation);
      //double expectation = random->sampleDouble(-minRewardExpectation, maxRewardExpectation);
      //double expectation = random->sampleDoubleFromGaussian(minRewardExpectation, maxRewardExpectation);

      double expectation = random->sampleBool(minRewardExpectation) ? 0.0 : random->sampleDouble(0.0, maxRewardExpectation);

      arms[i] = bernoulliSampler(juce::jlimit(0.0, 1.0, expectation));
    }

    std::vector<ArmInfo> armInfos(totalNumArms);

    // do episode
    std::vector<size_t> presentedArms;
    std::vector<size_t> bestArms;
    double sumOfRewards = 0.0;
    for (size_t t = 0; t < horizon; ++t)
    {
      size_t firstDocumentIndex = (t * (totalNumArms - numDocumentsAliveSimultaneously)) / horizon;
      jassert(firstDocumentIndex + numDocumentsAliveSimultaneously <= totalNumArms);


      // select arms
      std::vector<size_t> presentedArms;
      for (size_t i = 0; i < numDocumentsAliveSimultaneously; ++i)
        if (random->sampleBool(0.5))
        {
          size_t index = firstDocumentIndex + i;
          presentedArms.push_back(index);
          ++armInfos[index].presentedCount;
        }

      // compute scores for each arm and select best arms
      double bestScore = -DBL_MAX;
      bestArms.clear();
      for (size_t i = 0; i < presentedArms.size(); ++i)
      {
        size_t index = presentedArms[i];
        double score = computeScore(formula, armInfos[index], i / (double)presentedArms.size());
        if (score >= bestScore)
        {
          if (score > bestScore)
          {
            bestArms.clear();
            bestScore = score;
          }
          bestArms.push_back(index);
        }
      }
      if (presentedArms.empty())
        continue;
      
      // sample best arm
      size_t armIndex;
      if (bestArms.size())
        armIndex = bestArms[random->sampleSize(bestArms.size())];
      else
        armIndex = presentedArms[random->sampleSize(presentedArms.size())];
      
      // play arm
      double reward = arms[armIndex]->sample(context, random).toDouble();
      sumOfRewards += reward;
      armInfos[armIndex].stats.push(reward);
    }

    return 10000.0 * sumOfRewards / horizon;
  }

  double computeScore(GPExpressionPtr formula, ArmInfo& info, double relativeIndex) const
  {
    if (!info.stats.getCount())
      return DBL_MAX;

    double variables[4];
    variables[0] = (double)info.presentedCount;
    variables[1] = info.stats.getMean() / maxRewardExpectation; // tmp !!
    variables[2] = info.stats.getCount();
    variables[3] = relativeIndex;
    return formula->compute(variables);
  }

protected:
  friend class ExploChallengeFormulaObjectiveClass;

  size_t horizon;
  size_t totalNumArms;
  size_t numArmsPerRound;
  size_t numDocumentsAliveSimultaneously;
  double minRewardExpectation;
  double maxRewardExpectation;
};

typedef ReferenceCountedObjectPtr<ExploChallengeFormulaObjective> ExploChallengeFormulaObjectivePtr;

class ExploChallengeFormulaSearchProblem : public FormulaSearchProblem
{
public:
  ExploChallengeFormulaSearchProblem() {}

  virtual FunctionPtr getObjective() const
    {return objective;}

  virtual EnumerationPtr getVariables() const
    {return exploChallengeFormulaVariablesEnumeration;}

  virtual void getOperators(std::vector<GPPre>& unaryOperators, std::vector<GPOperator>& binaryOperators) const
  {
    for (size_t i = gpOpposite; i <= gpAbs; ++i)
      if (i != gpExp)
        unaryOperators.push_back((GPPre)i);
    for (size_t i = gpAddition; i <= gpMax; ++i)
      binaryOperators.push_back((GPOperator)i);
  }

  virtual void sampleInputs(ExecutionContext& context, size_t count, std::vector< std::vector<double> >& res) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    res.resize(count);

    for (size_t index = 0; index < count; ++index)
    {
      std::vector<double> input(4);
      input[0] = random->sampleSize(1, 1000); // presented count
      input[1] = juce::jmax(0.0, random->sampleDouble(-0.1, 1.1)); // reward mean
      input[2] = random->sampleSize(1, 1000); // play count
      input[3] = random->sampleDouble(); // relative index
      res[index] = input;
    }
  }

  struct ValueComparator
  {
    bool operator() (const std::pair<size_t, double>& left, const std::pair<size_t, double>& right) const
    {
      double l = isNumberValid(left.second) ? left.second : -DBL_MAX;
      double r = isNumberValid(right.second) ? right.second : -DBL_MAX;
      return (l != r ? l < r : left.first < right.first);
    }
  };

  virtual BinaryKeyPtr makeBinaryKey(const GPExpressionPtr& expression, const std::vector< std::vector<double> >& inputSamples) const
  {
    std::vector< std::pair<size_t, double> > scores(inputSamples.size());
    for (size_t i = 0; i < scores.size(); ++i)
    {
      scores[i].first = i;
      scores[i].second = expression->compute(&inputSamples[i][0]);
    }
    std::sort(scores.begin(), scores.end(), ValueComparator());

    BinaryKeyPtr res = new BinaryKey(scores.size() * 4);
    for (size_t i = 0; i < scores.size(); ++i)
      res->push32BitInteger(scores[i].first);
    return res;
  }

protected:
  friend class ExploChallengeFormulaSearchProblemClass;

  ExploChallengeFormulaObjectivePtr objective;
};


}; /* namespace lbcpp */

#endif // !LBCPP_GP_SEARCH_PROBLEM_EXPLO_CHALLENGE_FORMULA_H_
