/*-----------------------------------------.---------------------------------.
| Filename: BanditFormulaOptimizer.h       | Bandit Formula Optimizer        |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2011 19:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_BANDIT_FORMULA_OPTIMIZER_H_
# define LBCPP_SEQUENTIAL_DECISION_BANDIT_FORMULA_OPTIMIZER_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "LearningRuleFormulaSearchProblem.h"
# include "BanditFormulaSearchProblem.h"
# include <algorithm>
# include "../WorkUnit/GPSandBox.h"
# include "../Bandits/FindBanditsFormula.h"

namespace lbcpp
{

class FormulaPool
{
public:
  FormulaPool(DiscreteBanditPolicyPtr policy, FunctionPtr objective)
    : policy(policy), objective(objective) {}

  size_t playBestFormula(ExecutionContext& context)
  {
    if (formulas.empty())
      return (size_t)-1;

    size_t index = policy->selectNextBandit(context);

    GPExpressionPtr formula = formulas[index];
    WorkUnitPtr workUnit = functionWorkUnit(objective, std::vector<Variable>(1, formula));
   
    RegretScoreObjectPtr regret = context.run(workUnit, false).getObjectAndCast<RegretScoreObject>();
    receiveReward(index, regret->getReward(), regret->getRegret());
    //context.informationCallback(T("reward: ") + String(regret->getReward()) + T(" regret: ") + String(regret->getRegret()));
    return index;
  }
  
  void displayBestFormulas(ExecutionContext& context, size_t count, std::vector<GPExpressionPtr>& bestFormulas)
  {
    std::multimap<double, size_t> formulasByMeanReward;
    for (size_t i = 0; i < formulas.size(); ++i)
      formulasByMeanReward.insert(std::make_pair(policy->getRewardEmpiricalMean(i), i));

    size_t i = 1;
    bestFormulas.reserve(count);
    for (std::multimap<double, size_t>::reverse_iterator it = formulasByMeanReward.rbegin(); i < count && it != formulasByMeanReward.rend(); ++it, ++i)
    {
      GPExpressionPtr expression = formulas[it->second];
      bestFormulas.push_back(expression);
      size_t playedCount = policy->getBanditPlayedCount(it->second);
      String info = T("[") + String((int)i) + T("] ") + expression->toShortString()
          + T(" meanRegret = ") + String(formulaRegrets[it->second].getMean(), 3)
          + T(" meanReward = ") + String(it->first, 3)
          + T(" playedCount = ") + String((int)playedCount);

      context.enterScope(info);
      context.resultCallback(T("rank"), i);
      context.resultCallback(T("formula"), expression);
      context.resultCallback(T("meanRegret"), formulaRegrets[it->second].getMean());
      context.resultCallback(T("regretStddev"), formulaRegrets[it->second].getStandardDeviation());
      context.resultCallback(T("meanReward"), it->first);
      context.resultCallback(T("playedCount"), playedCount);
      context.leaveScope();
    }
  }

  void run(ExecutionContext& context, const std::vector<GPExpressionPtr>& formulas, size_t numIterations, size_t iterationsLength, const std::vector<double>* expectedRewards = NULL, std::vector<double>* simpleRegrets = NULL)
  {
    this->formulas = formulas;
    formulaRegrets.clear();
    formulaRegrets.resize(formulas.size());

    policy->initialize(formulas.size());

    double bestExpectedReward = -DBL_MAX;
    if (expectedRewards)
    {
      for (size_t i = 0; i < expectedRewards->size(); ++i)
        if ((*expectedRewards)[i] > bestExpectedReward)
          bestExpectedReward = (*expectedRewards)[i];
    }

    if (simpleRegrets)
      simpleRegrets->resize(numIterations);
    double cumulativeRegret = 0.0;

    for (size_t i = 0; i < numIterations; ++i)
    {
      if (numIterations > 1)
        context.enterScope(T("Iteration ") + String((int)i));

      for (size_t j = 0; j < iterationsLength; ++j)
      {
        size_t index = playBestFormula(context);
        if (expectedRewards)
          cumulativeRegret += bestExpectedReward - (*expectedRewards)[index];

        if (iterationsLength > 1000 && ((j+1) % 100 == 0))
          context.progressCallback(new ProgressionState(j + 1, iterationsLength, T("steps")));
      }

      context.resultCallback(T("iteration"), i);

      if (expectedRewards)
      {
        std::set<size_t> bests = getBestFormulaIndices();
        double simpleRegret = 0.0;
        if (bests.size())
        {
          size_t index = context.getRandomGenerator()->sampleSize(bests.size());
          std::set<size_t>::const_iterator it = bests.begin();
          for (size_t i = 0; i < index; ++i)
            ++it;
          simpleRegret = bestExpectedReward - (*expectedRewards)[*it];
        }
        else
          simpleRegret = 1.0;
        context.resultCallback(T("simpleRegret"), simpleRegret);
        context.resultCallback(T("cumulativeRegret"), cumulativeRegret);
        if (simpleRegrets)
          (*simpleRegrets)[i] = simpleRegret;
      }

      if (numIterations > 1)
      {
        context.leaveScope();
        context.progressCallback(new ProgressionState(i+1, numIterations, T("Iterations")));
      }
    }
  }

  std::vector<GPExpressionPtr> getBestFormulas(size_t count)
  {
    std::multimap<double, GPExpressionPtr> formulasByMeanReward;
    for (size_t i = 0; i < formulas.size(); ++i)
      formulasByMeanReward.insert(std::make_pair(policy->getRewardEmpiricalMean(i), formulas[i]));
    std::vector<GPExpressionPtr> res;
    for (std::multimap<double, GPExpressionPtr>::reverse_iterator it = formulasByMeanReward.rbegin(); res.size() < count && it != formulasByMeanReward.rend(); ++it)
      res.push_back(it->second);
    return res;
  }

  std::set<size_t> getBestFormulaIndices() const
  {
    std::set<size_t> bestIndices;
    double highestMean = -DBL_MAX;
    for (size_t i = 0; i < formulas.size(); ++i)
    {
      double mean = policy->getRewardEmpiricalMean(i);
      if (mean >= highestMean)
      {
        if (mean > highestMean)
        {
          bestIndices.clear();
          highestMean = mean;
        }
        bestIndices.insert(i);
      }
    }
    return bestIndices;
  }

protected:
  DiscreteBanditPolicyPtr policy;
  FunctionPtr objective;
  std::vector<GPExpressionPtr> formulas;
  std::vector<ScalarVariableStatistics> formulaRegrets;

  void receiveReward(size_t index, double reward, double regret)
  {
    policy->updatePolicy(index, reward);
    formulaRegrets[index].push(regret);
  }
};

/////////// Naive Monte Carlo ////////////////

class PlayFormulasNTimes : public WorkUnit
{
public:
  PlayFormulasNTimes() : numEstimations(1000) {}

  struct Result
  {
    Result(const GPExpressionPtr& formula)
      : formula(formula), regretStats(new ScalarVariableStatistics("regret")), rewardStats(new ScalarVariableStatistics("reward")) {}

    GPExpressionPtr formula;
    ScalarVariableStatisticsPtr regretStats;
    ScalarVariableStatisticsPtr rewardStats;
  };

  virtual Variable run(ExecutionContext& context)
  {   
    FunctionPtr objective = problem->getObjective();
    if (!objective->initialize(context, gpExpressionClass))
      return false;

    std::vector<GPExpressionPtr> formulas;
    if (!GPExpression::loadFormulasFromFile(context, formulasFile, problem->getVariables(), formulas))
      return false;
    context.informationCallback("We have " + String((int)formulas.size()) + " formulas");

    context.enterScope(T("Evaluating formulas ") + String((int)numEstimations) + T(" times"));

    typedef std::multimap<double, Result> SortedFormulasMap;

    SortedFormulasMap sortedFormulas;
    for (size_t i = 0; i < formulas.size(); ++i)
    {
      GPExpressionPtr formula = formulas[i];
      Result res(formula);
      for (size_t j = 0; j < numEstimations; ++j)
      {
        RegretScoreObjectPtr regret = objective->compute(context, formula).getObjectAndCast<RegretScoreObject>();
        res.regretStats->push(regret->getRegret());
        res.rewardStats->push(regret->getReward());
      }
      double score = res.regretStats->getMean();
      sortedFormulas.insert(std::make_pair(score, res));
      context.progressCallback(new ProgressionState((i+1), formulas.size(), T("Formulas")));
    }
    context.leaveScope();

    size_t i = 1;
    context.enterScope(T("Best formulas"));
    for (SortedFormulasMap::const_iterator it = sortedFormulas.begin(); it != sortedFormulas.end(); ++it)
    {
      const Result& res = it->second;

      context.enterScope(T("Formula ") + String((int)i) + T(": ") + res.formula->toShortString());
      context.resultCallback(T("rank"), i);
      context.resultCallback(T("rewardMean"), res.rewardStats->getMean());
      context.resultCallback(T("rewardStddev"), res.rewardStats->getStandardDeviation());
      context.resultCallback(T("regretMean"), res.regretStats->getMean());
      context.resultCallback(T("regretStddev"), res.regretStats->getStandardDeviation());
      context.resultCallback(T("formula"), res.formula);
      context.leaveScope(it->first);
      ++i;
    }
    context.leaveScope();

    if (outputFile != File::nonexistent)
    {
      context.enterScope(T("Saving to file ") + outputFile.getFileName());
      if (outputFile.exists())
        outputFile.deleteFile();
      OutputStream* ostr = outputFile.createOutputStream();
      if (ostr)
      {
        for (SortedFormulasMap::const_iterator it = sortedFormulas.begin(); it != sortedFormulas.end(); ++it)
        {
          const Result& res = it->second;
          *ostr << res.formula->toString()
                << " " << res.rewardStats->getMean() << " " << res.rewardStats->getStandardDeviation()
                << " " << res.regretStats->getMean() << " " << res.regretStats->getStandardDeviation() << "\n";
        }
        delete ostr;
      }
      context.leaveScope(true);
    }

    return true;
  }
  
protected:
  friend class PlayFormulasNTimesClass;

  FormulaSearchProblemPtr problem;
  File formulasFile;
  File outputFile;
  size_t numEstimations;
};

/////////// Compare search policies ///////

class CompareBanditFormulaSearchPolicies : public WorkUnit
{
public:
  CompareBanditFormulaSearchPolicies() : numRuns(10), numIterations(10), iterationsLength(0) {}

  struct Run : public WorkUnit
  {
    Run(juce::uint32 seed, const std::vector<GPExpressionPtr>& formulas, const DiscreteBanditPolicyPtr& policy, const FunctionPtr& objective, size_t numIterations, size_t iterationsLength, const std::vector<double>& formulaRewards)
      : seed(seed), formulas(formulas), policy(policy), objective(objective), numIterations(numIterations), iterationsLength(iterationsLength), formulaRewards(formulaRewards) {}

    virtual Variable run(ExecutionContext& context)
    {
      context.getRandomGenerator()->setSeed(seed);
      FormulaPool pool(policy->cloneAndCast<DiscreteBanditPolicy>(), objective);
      pool.run(context, formulas, numIterations, iterationsLength, &formulaRewards, &simpleRegrets);
      return true;
    }

    virtual String toShortString() const
      {return T("Run with seed ") + String((int)seed);}

    juce::uint32 seed;
    const std::vector<GPExpressionPtr>& formulas;
    DiscreteBanditPolicyPtr policy;
    FunctionPtr objective;
    size_t numIterations;
    size_t iterationsLength;
    const std::vector<double>& formulaRewards;

    std::vector<double> simpleRegrets;

    bool hasFoundBestBandit() const
      {return simpleRegrets.back() == 0.0;}
  };


  virtual Variable run(ExecutionContext& context)
  {   
    FunctionPtr objective = problem->getObjective();

    if (!objective->initialize(context, gpExpressionClass))
      return false;

    std::vector<GPExpressionPtr> formulas;
    std::vector<double> formulaRewards;
    if (!loadFormulasFromFile(context, formulasFile, problem->getVariables(), formulas, formulaRewards))
      return false;
    context.informationCallback("We have " + String((int)formulas.size()) + " formulas");

    if (!iterationsLength)
      iterationsLength = formulas.size();

    std::vector<DiscreteBanditPolicyPtr> policies;
    policies.push_back(ucb1TunedDiscreteBanditPolicy());
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(3.0, true));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(3.0, false));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(7.0, true));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(7.0, false));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(10.0, true));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(10.0, false));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(1.0, true, true));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(2.0, true, true));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(5.0, true, true));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(0.5, true, true));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(0.2, true, true));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(0.1, true, true));

  /*  policies.push_back(uniformDiscreteBanditPolicy());
    policies.push_back(greedyDiscreteBanditPolicy());
    policies.push_back(ucb1DiscreteBanditPolicy(2.0));
    policies.push_back(ucb1TunedDiscreteBanditPolicy());
    policies.push_back(klucbDiscreteBanditPolicy(0.0));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(1.0, true));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(1.0, false));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(2.0, true));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(2.0, false));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(5.0, true));
    policies.push_back(new Formula5IndexBasedDiscreteBanditPolicy(5.0, false));*/
    

    std::vector< std::vector<ScalarVariableStatistics> > simpleRegretStatistics(policies.size(), std::vector<ScalarVariableStatistics>(numIterations));

    for (size_t i = 0; i < policies.size(); ++i)
    {
      DiscreteBanditPolicyPtr policy = policies[i];
      context.enterScope(T("Policy ") + policy->toShortString());

      CompositeWorkUnitPtr workUnit(new CompositeWorkUnit(T("Making ") + String((int)numRuns) + T(" runs"), numRuns));
      for (size_t r = 0; r < numRuns; ++r)
        workUnit->setWorkUnit(r, new Run((juce::uint32)(1664 + 51 * r), formulas, policy, objective, numIterations, iterationsLength, formulaRewards));
      workUnit->setPushChildrenIntoStackFlag(true);
      context.run(workUnit);
      
      std::vector<ScalarVariableStatistics>& simpleRegretStats = simpleRegretStatistics[i];
      size_t numSuccess = 0;
      for (size_t r = 0; r < numRuns; ++r)
      {
        ReferenceCountedObjectPtr<Run> run = workUnit->getWorkUnit(r).staticCast<Run>();
        std::vector<double>& simpleRegrets = run->simpleRegrets;
        jassert(simpleRegrets.size() == numIterations);
        for (size_t j = 0; j < numIterations; ++j)
          simpleRegretStats[j].push(simpleRegrets[j]);
        if (run->hasFoundBestBandit())
          ++numSuccess;
      }
      context.informationCallback(T("Success rate: ") + String((int)numSuccess) + T(" / ") + String((int)numRuns));

      context.enterScope(T("Results"));
      for (size_t j = 0; j < numIterations; ++j)
      {
        context.enterScope(T("Iteration ") + String((int)j));
        context.resultCallback(T("iteration"), j);
        context.resultCallback(T("simpleRegret"), simpleRegretStats[j].getMean());
        context.resultCallback(T("simpleRegret stddev"), simpleRegretStats[j].getStandardDeviation());
        context.leaveScope();
      }
      context.leaveScope(simpleRegretStats.back().getMean());

      context.leaveScope(simpleRegretStats.back().getMean());
    }
    
    context.enterScope(T("All results"));
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));
      context.resultCallback(T("iteration"), i);
      for (size_t j = 0; j < policies.size(); ++j)
      {
        String name = policies[j]->toShortString();
        ScalarVariableStatistics& stats = simpleRegretStatistics[j][i];
        context.resultCallback(name, stats.getMean());
        context.resultCallback(name + T(" stddev"), stats.getStandardDeviation());
      }
      context.leaveScope();
    }
    context.leaveScope();    
    return true;
  }
  
protected:
  friend class CompareBanditFormulaSearchPoliciesClass;

  FormulaSearchProblemPtr problem;
  File formulasFile;
  size_t numRuns;
  size_t numIterations;
  size_t iterationsLength;

  bool loadFormulasFromFile(ExecutionContext& context, const File& formulasFile, EnumerationPtr variables, std::vector<GPExpressionPtr>& res, std::vector<double>& rewards)
  {
    InputStream* istr = formulasFile.createInputStream();
    if (!istr)
    {
      context.errorCallback(T("Could not open file ") + formulasFile.getFullPathName());
      return false;
    }
    while (!istr->isExhausted())
    {
      String line = istr->readNextLine();
      if (!line.isEmpty())
      {
        int position = 0;
        GPExpressionPtr expression = GPExpression::createFromString(context, line, variables, position);
        if (expression)
          res.push_back(expression);
        String remainder = line.substring(position).trim();
        int space = remainder.indexOfAnyOf(T(" \t\n\r"));
        double reward = remainder.substring(0, space).getDoubleValue();
        rewards.push_back(reward);
      }
    }
    delete istr;
    return true;
  }
};

/////////// The algorithm for discovering formulas /////////

class BanditFormulaSearch : public WorkUnit
{
public:
  BanditFormulaSearch() : numTimeSteps(100000), minHorizon(10), maxHorizon(10000), verbose(false) {}

  struct Run : public WorkUnit
  {
    Run(const std::vector<GPExpressionPtr>& formulas, FunctionPtr objective, size_t numTimeSteps, const String& description, const File& outputFile, double C, bool verbose)
      : formulas(formulas), objective(objective), numTimeSteps(numTimeSteps), description(description), outputFile(outputFile), C(C), verbose(verbose) {}

    virtual String toShortString() const
      {return description;}

    virtual Variable run(ExecutionContext& context)
    {
      DiscreteBanditPolicyPtr policy = new Formula5IndexBasedDiscreteBanditPolicy(C, false); // rk + C/tk
      FormulaPool pool(policy, objective);
      pool.run(context, formulas, 1, numTimeSteps);
      
      std::vector<GPExpressionPtr> bestFormulas;
      pool.displayBestFormulas(context, 100, bestFormulas);

      if (outputFile.existsAsFile())
        outputFile.deleteFile();
      OutputStream* ostr = outputFile.createOutputStream();
      if (ostr)
      {
        for (size_t i = 0; i < bestFormulas.size(); ++i)
          *ostr << bestFormulas[i]->toString() << "\n";
        ostr->flush();
        delete ostr;
      }
      return true;
    }
      
  protected:
    const std::vector<GPExpressionPtr>& formulas;
    FunctionPtr objective;
    size_t numTimeSteps;
    String description;
    File outputFile;
    double C;
    bool verbose;
  };

  virtual Variable run(ExecutionContext& context)
  {
    context.informationCallback("Problem sampler: " + problemSampler->toShortString());

    std::vector<GPExpressionPtr> formulas;
    EnumerationPtr variables = gpExpressionDiscreteBanditPolicyVariablesEnumeration;
    if (!GPExpression::loadFormulasFromFile(context, formulasFile, variables, formulas))
      return false;
    context.informationCallback("We have " + String((int)formulas.size()) + " formulas");
    
    if (!outputDirectory.exists())
      outputDirectory.createDirectory();

    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Running"));

    double Ccumul = 5.0;
    double Csimple = 0.5;
    size_t horizon = minHorizon;
    while (horizon <= maxHorizon)
    {
      String pre = "Horizon " + String((int)horizon);
      FunctionPtr objective = new BanditFormulaObjective(false, problemSampler, horizon);
      objective->initialize(context, gpExpressionClass);
      File outputFile = outputDirectory.getChildFile("cum" + String((int)horizon) + ".txt");
      workUnit->addWorkUnit(new Run(formulas, objective, numTimeSteps, pre + T(" Cumulative Regret"), outputFile, Ccumul, verbose));
      
      objective = new BanditFormulaObjective(true, problemSampler, horizon);
      objective->initialize(context, gpExpressionClass);
      outputFile = outputDirectory.getChildFile("sim" + String((int)horizon) + ".txt");
      workUnit->addWorkUnit(new Run(formulas, objective, numTimeSteps, pre + T(" Simple Regret"), outputFile, Csimple, verbose));

      if (minHorizon < 1000)
	      horizon *= 10;
      else
	      horizon += minHorizon;
    }
    
    workUnit->setPushChildrenIntoStackFlag(true);
    context.resultCallback(T("minHorizon"), minHorizon);
    context.resultCallback(T("maxHorizon"), maxHorizon);
    context.run(workUnit, false);
    return true;
  }

protected:
  friend class BanditFormulaSearchClass;

  File formulasFile;
  size_t numTimeSteps;
  BanditProblemSamplerPtr problemSampler;
  size_t minHorizon;
  size_t maxHorizon;
  File outputDirectory;
  bool verbose;
};

/////////// Evaluate discovered policies and compare against baselines /////////

class EvaluateBanditFormulas : public WorkUnit
{
public:
  EvaluateBanditFormulas() : horizon(1000), isSimpleRegret(false) {}

  virtual Variable run(ExecutionContext& context)
  {   
    context.informationCallback("Problem sampler: " + problemSampler->toShortString());

    sampleProblems(context, 10000, tuningStates);    
    sampleProblems(context, 10000, validationStates);

    // evaluate discovered formulas
    std::vector<GPExpressionPtr> formulas;
    EnumerationPtr variables = gpExpressionDiscreteBanditPolicyVariablesEnumeration;
    if (!GPExpression::loadFormulasFromFile(context, formulasFile, variables, formulas))
      return false;
    context.enterScope(T("Discovered formulas"));
    context.informationCallback("We have " + String((int)formulas.size()) + " formulas");
    double best = DBL_MAX;
    //    if (formulas.size() > 5)
    //  formulas.resize(5);
    for (size_t i = 0; i < formulas.size(); ++i)
      best = juce::jmin(best, evaluatePolicy(context, gpExpressionDiscreteBanditPolicy(formulas[i]), i));
    context.leaveScope(best);

    // evaluate baselines that have no parameters
    context.enterScope(T("Baselines without parameters"));
    best = DBL_MAX;
    best = juce::jmin(best, evaluatePolicy(context, uniformDiscreteBanditPolicy(), 0));
    best = juce::jmin(best, evaluatePolicy(context, greedyDiscreteBanditPolicy(), 1));
    best = juce::jmin(best, evaluatePolicy(context, ucb1TunedDiscreteBanditPolicy(), 2));
    context.leaveScope(best);

    // tune and evaluate baselines that have parameters
    context.enterScope(T("Baselines with parameters"));
    best = DBL_MAX;
    // single param
    best = juce::jmin(best, tuneAndEvaluatePolicy(context, ucb1DiscreteBanditPolicy(), 0));
    best = juce::jmin(best, tuneAndEvaluatePolicy(context, new Formula5IndexBasedDiscreteBanditPolicy(1, false), 1));
    best = juce::jmin(best, tuneAndEvaluatePolicy(context, new Formula5IndexBasedDiscreteBanditPolicy(1, true), 2));
    best = juce::jmin(best, tuneAndEvaluatePolicy(context, klucbDiscreteBanditPolicy(), 3));
    // two params
    best = juce::jmin(best, tuneAndEvaluatePolicy(context, ucbvDiscreteBanditPolicy(), 4));
    best = juce::jmin(best, tuneAndEvaluatePolicy(context, epsilonGreedyDiscreteBanditPolicy(), 5));
    context.leaveScope(best);
    return true;
  }

protected:
  friend class EvaluateBanditFormulasClass;

  File formulasFile;
  BanditProblemSamplerPtr problemSampler;
  size_t horizon;
  bool isSimpleRegret;

  std::vector<DiscreteBanditStatePtr> tuningStates;
  std::vector<DiscreteBanditStatePtr> validationStates;

  void sampleProblems(ExecutionContext& context, size_t count, std::vector<DiscreteBanditStatePtr>& res)
  {
    res.resize(count);
    for (size_t i = 0; i < count; ++i)
      res[i] = new DiscreteBanditState(problemSampler->sampleArms(context.getRandomGenerator()));
  }

  double tuneAndEvaluatePolicy(ExecutionContext& context, DiscreteBanditPolicyPtr policy, size_t index) const
  {
    context.enterScope(T("Tuning ") + policy->toShortString());

    DiscreteBanditPolicyPtr bestPolicy;
    double bestScore = DBL_MAX;

    Parameterized* p = Parameterized::get(policy);
    jassert(p);
    if (p->getParametersType() == doubleType)
    {
      double cMin, cMax, cStep;
      if (policy->getClassName() == T("KLUCBDiscreteBanditPolicy"))
        cMin = -3.0, cMax = 3.0, cStep = 0.3;
      else if (policy->getClassName() == T("Formula5IndexBasedDiscreteBanditPolicy"))
        cMin = 0.0, cMax = 10.0, cStep = 0.5;
      else
        cMin = 0.0, cMax = 2.0, cStep = 0.1;

      for (double c = cMin; c <= cMax; c += cStep)
      {
        context.enterScope(T("C = ") + String(c));
        context.resultCallback(T("c"), c);
        DiscreteBanditPolicyPtr pol = Parameterized::cloneWithNewParameters(policy, c);
        double score = evaluatePolicy(context, pol, 0, false);
        context.resultCallback(T("regret"), score);
        if (score < bestScore)
        {
          bestScore = score;
          bestPolicy = pol;
        }
        context.leaveScope(score);
      }
    }
    else
    {
      jassert(p->getParametersType() == pairClass(doubleType, doubleType));
      for (double c1 = 0.0; c1 <= 2.0; c1 += 0.1)
        for (double c2 = 0.0; c2 <= 2.0; c2 += 0.1)
        {
          context.enterScope(T("C1 = ") + String(c1) + T(" C2 = ") + String(c2));
          context.resultCallback(T("c1"), c1);
          context.resultCallback(T("c2"), c2);
          DiscreteBanditPolicyPtr pol = Parameterized::cloneWithNewParameters(policy, new Pair(c1, c2));
          double score = evaluatePolicy(context, pol, 0, false);
          context.resultCallback(T("regret"), score);
          if (score < bestScore)
          {
            bestScore = score;
            bestPolicy = pol;
          }
          context.leaveScope(score);
        }
    }

    context.leaveScope(bestScore);

    return bestPolicy ? evaluatePolicy(context, bestPolicy, index) : DBL_MAX;
  }

  double evaluatePolicy(ExecutionContext& context, DiscreteBanditPolicyPtr policy, size_t index, bool verbose = true) const
  {
    if (verbose)
    {
      context.enterScope(T("Evaluating ") + policy->toShortString());
      context.resultCallback(T("index"), index);
    }

    ScalarVariableStatistics regretStats;
    for (size_t j = 0; j < validationStates.size(); ++j)
    {
      const std::vector<SamplerPtr>& arms = validationStates[j]->getArms();
      FunctionPtr objective = new BanditFormulaObjective(isSimpleRegret, new ConstantBanditProblemSampler(arms), horizon);
      RegretScoreObjectPtr regret = objective->compute(context, policy).getObjectAndCast<RegretScoreObject>();
      regretStats.push(regret->getRegret());
    }

    if (verbose)
    {
      context.resultCallback(T("meanRegret"), regretStats.getMean());
      context.resultCallback(T("regretStddev"), regretStats.getStandardDeviation());
      context.resultCallback(T("playedCount"), (size_t)regretStats.getCount());
      context.resultCallback(T("policy"), policy);
      context.leaveScope(regretStats.getMean());
    }
    return regretStats.getMean();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_BANDIT_FORMULA_BANDIT_H_
