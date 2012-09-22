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
  ExploChallengeFormulaObjective(size_t horizon = 307000, DenseDoubleVectorPtr parameters = DenseDoubleVectorPtr())
    : SimpleUnaryFunction(gpExpressionClass, doubleType), horizon(horizon), parameters(parameters)
  {
  }

  static const double* getInitialSamplerParameters()
  {
    static const double meanAndStddevs[] = {
        0.35, 0.1,
        1.0, 0.5,
        0.35, 0.1,
        0.9, 0.1,
        0.0, 0.01,
        0.115, 0.1,
        0.0, 0.2,
        0.6, 0.2,
    };
    return meanAndStddevs;
  }

  static DenseDoubleVectorPtr getInitialGuess()
  {
    const double* params = getInitialSamplerParameters();
    DenseDoubleVectorPtr res = new DenseDoubleVector(exploChallengeFormulaObjectiveParametersEnumeration, doubleType);
    for (size_t i = 0; i < res->getNumValues(); ++i)
      res->setValue(i, params[i * 2]);
    return res;
  }

  static void applyConstraints(DenseDoubleVectorPtr params)
  {
    params->setValue(0, juce::jlimit(0.0, 1.0, params->getValue(0)));
    params->setValue(1, 1.0);//juce::jmax(0.0, params->getValue(1)));
    params->setValue(2, juce::jlimit(0.0, 1.0, params->getValue(2)));
    params->setValue(3, juce::jlimit(0.0, 1.0, params->getValue(3)));
    params->setValue(4, 0.0);//juce::jlimit(0.0, 1.0, params->getValue(4)));
    params->setValue(5, 0.126);//juce::jlimit(0.0, 1.0, params->getValue(5)));
    params->setValue(6, 0.0);
    params->setValue(7, 0.597);
  }

  struct ArmInfo
  {
    ArmInfo() : creationTime(0), rewardExpectation(0.0), rewardExpectationDecreaseRate(0.0), presentedCount(0), prevScore(1.0) {}

    int creationTime;
    double rewardExpectation;
    double rewardExpectationDecreaseRate;

    double sampleReward(RandomGeneratorPtr random) const
      {return random->sampleBool(juce::jlimit(0.0, 1.0, rewardExpectation - rewardExpectationDecreaseRate * presentedCount)) ? 1.0 : 0.0;}

    size_t presentedCount;
    ScalarVariableStatistics stats;
    double prevScore;
  };

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    if (!parameters)
    {
      // tuned for H=10000, N=20
      DenseDoubleVectorPtr parameters = new DenseDoubleVector(exploChallengeFormulaObjectiveParametersEnumeration, doubleType);
      parameters->setValue(0, 0.365); // probability of creation 0.334%
      parameters->setValue(1, 1.0); // ten arms
      parameters->setValue(2, 0.357); // min life time
      parameters->setValue(3, 0.842); // max life time
      parameters->setValue(4, 0.0);  // min reward
      parameters->setValue(5, 0.126); // max reward
      parameters->setValue(6, 0.0); // min reward decrease
      parameters->setValue(7, 0.597); // max reward decrease
      const_cast<ExploChallengeFormulaObjective* >(this)->parameters = parameters;
    }

    double probabilityOfNewArm = parameters->getValue(0) / 100.0;
    size_t numSelectedArms = (size_t)(juce::jmax(2.0, parameters->getValue(1) * 10.0));
    double minLifeTime = parameters->getValue(2) * horizon;
    double maxLifeTime = parameters->getValue(3) * horizon;
    double minRewardExpectation = -parameters->getValue(4);
    double maxRewardExpectation = parameters->getValue(5);
    double minRewardDecreaseRate = parameters->getValue(6) / horizon;
    double maxRewardDecreaseRate = parameters->getValue(7) / horizon;

    RandomGeneratorPtr random = context.getRandomGenerator();
 
    GPExpressionPtr formula = input.getObjectAndCast<GPExpression>();

    // do episode
    std::vector<ArmInfo> arms;
    arms.reserve((size_t)(1.2 * horizon * probabilityOfNewArm));
    std::map<int, size_t> alifeArms;

    std::vector<size_t> selectedArms;
    std::vector<size_t> bestArms;
    double sumOfRewards = 0.0;
    for (int t = -(int)horizon; t < (int)horizon; ++t)
    {
      if (random->sampleBool(probabilityOfNewArm))
      {
        // create new arm
        ArmInfo arm;
        arm.creationTime = t;
        int length = (int)(horizon * random->sampleDouble(minLifeTime, maxLifeTime));
        int deathTime = t + length;
        arm.rewardExpectation = juce::jlimit(0.0, 1.0, random->sampleDouble(minRewardExpectation, maxRewardExpectation));
        arm.rewardExpectationDecreaseRate = random->sampleDouble(minRewardDecreaseRate, maxRewardDecreaseRate);
        alifeArms[deathTime] = arms.size();
        arms.push_back(arm);
      }

      selectedArms.clear();
      std::map<int, size_t>::iterator it, nxt;
      for (it = alifeArms.begin(); it != alifeArms.end(); it = nxt)
      {
        nxt = it; ++nxt;
        if (it->first <= t)
          alifeArms.erase(it); // destroy dead arms
        else if (t >= 0)
          selectedArms.push_back(it->second); // select arm
      }

      if (t < 0)
        continue;

      if (selectedArms.size() < 2)
      {
        //std::cout << "no selected arms" << " params = " << parameters->toShortString() << std::endl;
        return -10000.0; // invalid setting: no documents at all
      }

      // randomly selected arms subset and sort arms
      std::random_shuffle(selectedArms.begin(), selectedArms.end());
      size_t s = (size_t)juce::jmin((int)selectedArms.size(), (int)numSelectedArms);
      std::set<size_t> tmp;
      for (size_t i = 0; i < s; ++i)
        tmp.insert(selectedArms[i]);
      selectedArms.clear();
      for (std::set<size_t>::const_iterator it = tmp.begin(); it != tmp.end(); ++it)
        selectedArms.push_back(*it);
      
      // compute scores for each arm and select best arms
      double bestScore = -DBL_MAX;
      bestArms.clear();
      for (size_t i = 0; i < selectedArms.size(); ++i)
      {
        size_t index = selectedArms[i];
        ArmInfo& arm = arms[index];
        arm.presentedCount++;
        double score = computeScore(formula, arm, i / (double)selectedArms.size());
        arm.prevScore = score;
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
      
      // sample best arm
      size_t armIndex;
      if (bestArms.size())
        armIndex = bestArms[random->sampleSize(bestArms.size())];
      else
        armIndex = selectedArms[random->sampleSize(selectedArms.size())];
      
      // play arm
      ArmInfo& playedArm = arms[armIndex];
      double reward = playedArm.sampleReward(random);
      sumOfRewards += reward;
      playedArm.stats.push(reward);
    }

    //std::cout << "Success evaluation: " << 10000.0 * sumOfRewards / horizon << std::endl;
    return 10000.0 * sumOfRewards / horizon;
  }

  double computeScore(GPExpressionPtr formula, ArmInfo& info, double relativeIndex) const
  {
    if (!info.stats.getCount())
      return DBL_MAX;

    double variables[4];
    variables[0] = (double)info.presentedCount;
    variables[1] = info.stats.getMean() * 10.0; // !!!!
    variables[2] = info.stats.getCount();
    variables[3] = relativeIndex;
    return formula->compute(variables);
  }

protected:
  friend class ExploChallengeFormulaObjectiveClass;

  size_t horizon;
  DenseDoubleVectorPtr parameters;
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
      input[3] = random->sampleDouble(-1000, 1000); // prev value
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
