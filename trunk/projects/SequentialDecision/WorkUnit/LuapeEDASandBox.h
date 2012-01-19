/*-----------------------------------------.---------------------------------.
| Filename: LuapeEDASandBox.h              | An EDA for LuapeNodes           |
| Author  : Francis Maes                   |                                 |
| Started : 19/01/2012 18:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_EDA_SAND_BOX_H_
# define LBCPP_LUAPE_EDA_SAND_BOX_H_

# include <lbcpp/Luape/LuapeBatchLearner.h>
# include <lbcpp/Luape/LuapeLearner.h>

namespace lbcpp
{

class EvaluateHIVLuapePolicyFunction : public SimpleUnaryFunction
{
public:
  EvaluateHIVLuapePolicyFunction(size_t horizon = 300)
    : SimpleUnaryFunction(luapeRegressorClass, doubleType), horizon(horizon) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    LuapeRegressorPtr regressor = input.getObjectAndCast<LuapeRegressor>();
    
    RandomGeneratorPtr random = context.getRandomGenerator();
  
    std::vector<double> initialState(6);
    initialState[0] = 163573;
    initialState[1] = 5;
    initialState[2] = 11945;
    initialState[3] = 46;
    initialState[4] = 63919;
    initialState[5] = 24;
    //for (size_t i = 0; i < initialState.size(); ++i)
    //  initialState[i] *= random->sampleDouble(0.8, 1.2);

    HIVDecisionProblemStatePtr state = new HIVDecisionProblemState(initialState);
    double cumulativeReward = 0.0;
    static const double discount = 0.98;
    for (size_t t = 0; t < horizon; ++t)
    {
      ContainerPtr actions = state->getAvailableActions();
      HIVDecisionProblemStatePtr nextStates[4];
      double rewards[4];
      double bestScore = -DBL_MAX;
      HIVDecisionProblemStatePtr nextState;
      double reward = 0.0;
      for (size_t j = 0; j < 4; ++j)
      {
        nextStates[j] = state->cloneAndCast<HIVDecisionProblemState>();
        Variable action = actions->getElement(j);
        nextStates[j]->performTransition(context, action, rewards[j], NULL);
        double score = regressor->compute(context, nextStates[j], doubleMissingValue).getDouble();
        if (score > bestScore)
          bestScore = score, nextState = nextStates[j], reward = rewards[j];
      }
      if (!nextState)
        return 0.0;
      state = nextState;
      cumulativeReward += pow(discount, (double)t) * reward; 
    }

    return cumulativeReward;
  }

protected:
  friend class EvaluateHIVLuapePolicyFunctionClass;

  size_t horizon;
};

class LuapeEDASandBox : public WorkUnit
{
public:
  LuapeEDASandBox() : complexity(4), populationSize(1000), numBests(100), numIterations(100) {}

  struct PopulationComparator
  {
    static size_t size(const LuapeNodePtr& node)
    {
      size_t res = 1;
      for (size_t i = 0; i < node->getNumSubNodes(); ++i)
        res += size(node->getSubNode(i));
      return res;
    }

    bool operator() (const std::pair<LuapeNodePtr, double>& left, const std::pair<LuapeNodePtr, double>& right) const
    {
      if (left.second != right.second)
        return left.second > right.second;
      size_t s1 = size(left.first);
      size_t s2 = size(right.first);
      if (s1 != s2)
        return s1 < s2;
      return left.first < right.first;
    }
  };

  virtual Variable run(ExecutionContext& context)
  {
    FunctionPtr objective = new EvaluateHIVLuapePolicyFunction(300);


    LuapeRegressorPtr regressor = new LuapeRegressor();
    if (!regressor->initialize(context, hivDecisionProblemStateClass, doubleType))
      return false;
    regressor->addInput(hivDecisionProblemStateClass, "s");
    //regressor->addFunction(andBooleanLuapeFunction());
    //regressor->addFunction(equalBooleanLuapeFunction());
    regressor->addFunction(addDoubleLuapeFunction());
    regressor->addFunction(subDoubleLuapeFunction());
    regressor->addFunction(mulDoubleLuapeFunction());
    regressor->addFunction(divDoubleLuapeFunction());
    regressor->addFunction(logDoubleLuapeFunction());
    regressor->addFunction(sqrtDoubleLuapeFunction());
    //regressor->addFunction(greaterThanDoubleLuapeFunction());
    regressor->addFunction(getVariableLuapeFunction());
    regressor->addFunction(new GetDecisionProblemSuccessorState(4));

    std::vector< std::pair<LuapeNodePtr, double> > population;
    std::set<LuapeNodePtr> processedNodes;

    LuapeNodeBuilderPtr nodeBuilder = biasedRandomSequentialNodeBuilder(populationSize / 4, complexity, 0.0);
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i));
      context.resultCallback("iteration", i);
     
      context.enterScope(T("Generating candidates"));
      std::vector<LuapeNodePtr> candidates;
      for (size_t trial = 0; trial < 100 && candidates.size() < populationSize; ++trial)
      {
        std::vector<LuapeNodePtr> c;
        nodeBuilder->buildNodes(context, regressor, 0, c);
        for (size_t i = 0; i < c.size(); ++i)
          if (processedNodes.find(c[i]) == processedNodes.end())
          {
            context.informationCallback(T("Candidate: ") + c[i]->toShortString());
            processedNodes.insert(c[i]);
            candidates.push_back(c[i]);
          }
      }
      if (candidates.size() > populationSize)
        candidates.erase(candidates.begin() + populationSize, candidates.end());
      context.leaveScope(candidates.size());

      context.enterScope(T("Evaluating candidates"));
      population.reserve(population.size() + candidates.size());
      size_t numEvaluated = 0;
      for (size_t j = 0; j < candidates.size(); ++j)
      {
        LuapeNodePtr candidate = candidates[j];
        regressor->setRootNode(context, candidate);
        double score = objective->compute(context, regressor).getDouble();
        population.push_back(std::make_pair(candidate, score));
        context.progressCallback(new ProgressionState(j+1, candidates.size(), T("Candidates")));
        ++numEvaluated;
      }
      regressor->setRootNode(context, LuapeNodePtr());
      context.leaveScope(numEvaluated);

      context.enterScope(T("Updating population"));
      std::sort(population.begin(), population.end(), PopulationComparator());
      if (population.size() > populationSize)
        population.erase(population.begin() + populationSize, population.end());
      for (size_t j = 0; j < population.size(); ++j)
        population[j].first->addImportance(1.0 / (j + 1.0));
      context.leaveScope(population.front().second);

      context.resultCallback(T("bestScore"), population.front().second);
      context.resultCallback(T("worstScore"), population.back().second);

      for (size_t i = 0; i < 10; ++i)
      {
        if (i >= population.size())
          break;
        context.informationCallback(T("Top ") + String((int)i + 1) + T(": ") + population[i].first->toShortString() + T(" [") + Variable(population[i].second).toShortString() + T("]"));
      }

      context.leaveScope(population.front().second);
    }

    return true;
  }

protected:
  friend class LuapeEDASandBoxClass;

  size_t complexity;
  size_t populationSize;
  size_t numBests;
  size_t numIterations;
};


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_EDA_SAND_BOX_H_

